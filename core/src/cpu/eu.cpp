#include "eu.hpp"
#include "arch.hpp"

#include "instructions.hpp"

using namespace E5150::I8086;

static bool EUWaitSuccessfullDecodeClock(void);
static unsigned int INSTRUCTION_CLOCK_LEFT = 0;

static void printCurrentInstruction(void)
{
	#if defined(SEE_CURRENT_INST) || defined(SEE_ALL)
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	if (!inst)
		return;
	std::cout << std::hex << cpu.cs << ":" << cpu.ip << " (" << cpu.genAddress(cpu.cs,cpu.ip) << ")" << std::dec << ": ";
	std::cout << xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(&cpu.eu.decodedInst)) << " : length = " << xed_decoded_inst_get_length(&cpu.eu.decodedInst) << std::endl;
	std::cout << xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(&cpu.eu.decodedInst)) << " ";
	unsigned int realOperandPos = 0;
	bool foundPtr = false;

	for (unsigned int i = 0; i < xed_decoded_inst_noperands(&cpu.eu.decodedInst); ++i)
	{
		const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, i));
		const xed_operand_visibility_enum_t op_vis = xed_operand_operand_visibility(xed_inst_operand(inst, i));

		if (true/*op_vis == XED_OPVIS_EXPLICIT*/)
		{
			if (foundPtr)
			{
				std::cout << ":";
				foundPtr = false;
			}
			else
			{
				if (realOperandPos > 0)
					std::cout << ", ";
			}

			switch (op_name)
			{
			case XED_OPERAND_RELBR:
				std::cout << (xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst) & 0xFFFF);
				break;

			case XED_OPERAND_PTR:
				std::cout << std::hex << (xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst) & 0xFFFF) << std::dec;
				foundPtr = true;
				break;

			case XED_OPERAND_REG0:
			case XED_OPERAND_REG1:
			case XED_OPERAND_REG2:
				std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
				break;

			case XED_OPERAND_IMM0:
			case XED_OPERAND_IMM1:
				std::cout << std::hex << (xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst) & 0xFFFF) << std::dec;
				break;

			//Displaying memory operand with format SEG:[[BASE +] [INDEX +] DISPLACEMENT ]
			case XED_OPERAND_MEM0:
			{
				const xed_reg_enum_t baseReg = xed_decoded_inst_get_base_reg(&cpu.eu.decodedInst, 0);
				const xed_reg_enum_t indexReg = xed_decoded_inst_get_index_reg(&cpu.eu.decodedInst, 0);
				const int64_t memDisplacement = xed_decoded_inst_get_memory_displacement(&cpu.eu.decodedInst,0);
				std::cout << ((xed_decoded_inst_get_memory_operand_length(&cpu.eu.decodedInst, 0) == 1) ? "BYTE" : "WORD") << " ";
				std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_seg_reg(&cpu.eu.decodedInst, 0)) << ":[";

				if (baseReg != XED_REG_INVALID)
					std::cout << xed_reg_enum_t2str(baseReg);
				
				if (indexReg != XED_REG_INVALID)
				{
					if (baseReg != XED_REG_INVALID)
						std::cout << " + ";
					std::cout << xed_reg_enum_t2str(indexReg);
				}

				if ((indexReg != XED_REG_INVALID) || (baseReg != XED_REG_INVALID))
				{
					if (memDisplacement != 0)
					{
						if (memDisplacement > 0)
							std::cout << " + " << memDisplacement;
						else
							std::cout << " - " << -memDisplacement;
					}
				}
				else
					std::cout << memDisplacement;
				std::cout << "]";
			}	break;

			default:
				break;
			}

			++realOperandPos;
		}
	}
	#endif
}

static void printRegisters(void)
{
#if defined(SEE_REGS) ||  defined(SEE_ALL)
	printf("CS: %#6x   DS: %#6x   ES: %#6x   SS: %#6x\n",cpu.cs,cpu.ds,cpu.es,cpu.ss);
	printf("AX: %#6x   bx: %#6x   CX: %#6x   DX: %#6x\n",cpu.ax,cpu.bx,cpu.cx,cpu.dx);
	printf("SI: %#6x   DI: %#6x   BP: %#6x   SP: %#6x\n\n",cpu.si,cpu.di,cpu.bp,cpu.sp);
#endif
}

static void printFlags(void)
{
#if defined(SEE_FLAGS) || defined(SEE_ALL)
	printf("%s  %s  %s  %s  %s  %s  %s  %s  %s\n",(cpu.flags & CPU::OVER) ? "OF" : "of", (cpu.flags & CPU::DIR) ? "DF" : "df", (cpu.flags & CPU::INTF) ? "IF" : "if", (cpu.flags & CPU::TRAP) ? "TF" : "tf", (cpu.flags & CPU::SIGN) ? "SF" : "sf", (cpu.flags & CPU::ZERRO) ? "ZF" : "zf", (cpu.flags & CPU::A_CARRY) ? "AF" : "af", (cpu.flags & CPU::PARRITY) ? "PF" : "pf", (cpu.flags & CPU::CARRY) ? "CF" : "cf");
#endif
}

static bool EUExecInstructionClock(void)
{
	#ifdef SEE_CURRENT_INST
		printCurrentInstruction();   printf(" clock left : %d\n",INSTRUCTION_CLOCK_LEFT);
	#endif
	if (INSTRUCTION_CLOCK_LEFT == 0)
	{
		cpu.eu.instructionFunction();
		cpu.eu.nextClockFunction = EUWaitSuccessfullDecodeClock;
		printRegisters();
		printFlags();
		return true;
	}

	INSTRUCTION_CLOCK_LEFT -= 1;
	return false;
}

//TODO: How to handle REP, LOCK, WAIT and ESC
static unsigned int prepareInstructionExecution(void)
{
	//At the end of the instructions that access memory there is w bit = 0 for byte operand and 1 one for word operands.
	//If this bit = 0 there is 1 memory access and if it = 1, 2 memory access
	const unsigned int nPrefix = xed_decoded_inst_get_nprefixes(&cpu.eu.decodedInst);
	cpu.eu.operandSizeWord = cpu.biu.instructionBufferQueue[nPrefix] & 0b1;
	const unsigned int memoryByteAccess = xed_decoded_inst_number_of_memory_operands(&cpu.eu.decodedInst) * (cpu.eu.operandSizeWord + 1);
	//printf("Computed by EU : Memory byte access : %d\n",memoryByteAccess);
	cpu.biu.requestMemoryByte(memoryByteAccess);
	switch (xed_decoded_inst_get_iclass(&cpu.eu.decodedInst))
	{
		case XED_ICLASS_MOV:
			cpu.eu.instructionFunction = MOV;
			return getMOVCycles();

		case XED_ICLASS_PUSH:
			cpu.eu.instructionFunction = PUSH;
			return getPUSHCycles();

		case XED_ICLASS_POP:
			cpu.eu.instructionFunction = POP;
			return getPOPCycles();

		case XED_ICLASS_XCHG:
			cpu.eu.instructionFunction = XCHG;
			return getXCHGCycles();

		case XED_ICLASS_IN:
			cpu.eu.instructionFunction = IN;
			return getINCycles();

		case XED_ICLASS_OUT:
			cpu.eu.instructionFunction = OUT;
			return getOUTCycles();

		case XED_ICLASS_XLAT:
			cpu.eu.instructionFunction = XLAT;
			return getXLATCycles();

		case XED_ICLASS_LEA:
			cpu.eu.instructionFunction = LEA;
			return getLEACycles();

		case XED_ICLASS_LDS:
			cpu.eu.instructionFunction = LDS;
			return getLDSCycles();

		case XED_ICLASS_LES:
			cpu.eu.instructionFunction = LES;
			return getLESCycles();

		case XED_ICLASS_LAHF:
			cpu.eu.instructionFunction = LAHF;
			return getLAHFCycles();

		case XED_ICLASS_SAHF:
			cpu.eu.instructionFunction = SAHF;
			return getSAHFCycles();

		case XED_ICLASS_PUSHF:
			cpu.eu.instructionFunction = PUSHF;
			return getPUSHFCycles();

		case XED_ICLASS_POPF:
			cpu.eu.instructionFunction = POPF;
			return getPOPFCycles();

		case XED_ICLASS_ADD:
			cpu.eu.instructionExtraData.withCarry = false;
			cpu.eu.instructionFunction = ADD;
			return getADDCycles();

		case XED_ICLASS_ADC:
			cpu.eu.instructionExtraData.withCarry = true;
			cpu.eu.instructionFunction = ADD;
			return getADDCycles();

		case XED_ICLASS_INC:
			cpu.eu.instructionFunction = INC;
			return getINCCycles();

		case XED_ICLASS_AAA:
			cpu.eu.instructionFunction = AAA;
			return getAAACycles();

		case XED_ICLASS_DAA:
			cpu.eu.instructionFunction = DAA;
			return getDAACycles();

		case XED_ICLASS_SUB:
			cpu.eu.instructionExtraData.withCarry = false;
			cpu.eu.instructionFunction = SUB;
			return getSUBCycles();

		case XED_ICLASS_SBB:
			cpu.eu.instructionExtraData.withCarry = true;
			cpu.eu.instructionFunction = SUB;
			return getSUBCycles();

		case XED_ICLASS_DEC:
			cpu.eu.instructionFunction = DEC;
			return getDECCycles();

		case XED_ICLASS_NEG:
			cpu.eu.instructionFunction = NEG;
			return getNEGCycles();

		case XED_ICLASS_CMP:
			cpu.eu.instructionFunction = CMP;
			return getCMPCycles();

		case XED_ICLASS_AAS:
			cpu.eu.instructionFunction = AAS;
			return getAASCycles();

		case XED_ICLASS_DAS:
			cpu.eu.instructionFunction = DAS;
			return getDASCycles();

		case XED_ICLASS_MUL:
			cpu.eu.instructionExtraData.isSigned = false;
			cpu.eu.instructionFunction = MUL;
			return getMULCycles();

		case XED_ICLASS_IMUL:
			cpu.eu.instructionExtraData.isSigned = true;
			cpu.eu.instructionFunction = MUL;
			return getIMULCycles();

		case XED_ICLASS_DIV:
			cpu.eu.instructionExtraData.isSigned = false;
			cpu.eu.instructionFunction = DIV;
			return getDIVCycles();

		case XED_ICLASS_IDIV:
			cpu.eu.instructionExtraData.isSigned = true;
			cpu.eu.instructionFunction = DIV;
			return getIDIVCycles();

		case XED_ICLASS_AAD:
			cpu.eu.instructionFunction = AAD;
			return getAADCycles();

		case XED_ICLASS_CBW:
			cpu.eu.instructionFunction = CBW;
			return getCBWCycles();

		case XED_ICLASS_CWD:
			cpu.eu.instructionFunction = CWD;
			return getCWDCycles();

		case XED_ICLASS_NOT:
			cpu.eu.instructionFunction = NOT;
			return getNOTCycles();

		case XED_ICLASS_SHL:
			cpu.eu.instructionExtraData.setDirectionIsLeft();
			cpu.eu.instructionFunction = SHIFT;
			return getSHIFT_ROTATECycles();

		case XED_ICLASS_SHR:
			cpu.eu.instructionFunction = SHIFT;
			return getSHIFT_ROTATECycles();

		case XED_ICLASS_SAR:
			cpu.eu.instructionExtraData.setInstructionIsArithmetic();
			cpu.eu.instructionFunction = SHIFT;
			return getSHIFT_ROTATECycles();

		case XED_ICLASS_ROL:
			cpu.eu.instructionExtraData.setDirectionIsLeft();
			cpu.eu.instructionFunction = ROTATE;
			return getSHIFT_ROTATECycles();

		case XED_ICLASS_ROR:
			cpu.eu.instructionFunction = ROTATE;
			return getSHIFT_ROTATECycles();

		case XED_ICLASS_RCL:
			cpu.eu.instructionExtraData.setRotationWithCarry();
			cpu.eu.instructionExtraData.setDirectionIsLeft();
			cpu.eu.instructionFunction = ROTATE;
			return getSHIFT_ROTATECycles();

		case XED_ICLASS_RCR:
			cpu.eu.instructionExtraData.setRotationWithCarry();
			cpu.eu.instructionFunction = ROTATE;
			return getSHIFT_ROTATECycles();

		case XED_ICLASS_AND:
			cpu.eu.instructionFunction = AND;
			return getANDCycles();

		case XED_ICLASS_TEST:
			cpu.eu.instructionFunction = TEST;
			return getTESTCycles();

		case XED_ICLASS_OR:
			cpu.eu.instructionFunction = OR;
			return getORCycles();

		case XED_ICLASS_XOR:
			cpu.eu.instructionFunction = XOR;
			return getXORCycles();

		case XED_ICLASS_MOVSB:
		case XED_ICLASS_MOVSW:
			cpu.eu.instructionFunction = MOVS;
			return getMOVSCycles();

		case XED_ICLASS_CMPSB:
		case XED_ICLASS_CMPSW:
			cpu.eu.instructionFunction = CMPS;
			return getCMPSCycles();

		case XED_ICLASS_SCASB:
		case XED_ICLASS_SCASW:
			cpu.eu.instructionFunction = SCAS;
			return getSCASCycles();

		case XED_ICLASS_LODSB:
		case XED_ICLASS_LODSW:
			cpu.eu.instructionFunction = LODS;
			return getLODSCycles();

		case XED_ICLASS_STOSB:
		case XED_ICLASS_STOSW:
			cpu.eu.instructionFunction = STOS;
			return getSTOSCycles();

		case XED_ICLASS_CALL_NEAR:
			cpu.eu.instructionFunction = CALL_NEAR;
			return getCALL_NEARCycles();

		case XED_ICLASS_CALL_FAR:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = CALL_FAR;
			return getCALL_FARCycles();

		case XED_ICLASS_JMP:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JMP_NEAR;
			return getJMP_NEARCycles();

		case XED_ICLASS_JMP_FAR:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JMP_FAR;
			return getJMP_FARCycles();

		case XED_ICLASS_RET_NEAR:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = RET_NEAR;
			return getRET_NEARCycles();

		case XED_ICLASS_RET_FAR:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = RET_FAR;
			return getRET_FARCycles();

		case XED_ICLASS_JZ:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JZ;
			return getJZCycles();

		case XED_ICLASS_JL:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JL;
			return getJLCycles();

		case XED_ICLASS_JLE:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JLE;
			return getJLECycles();

		case XED_ICLASS_JB:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JB;
			return getJBCycles();

		case XED_ICLASS_JBE:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JBE;
			return getJBECycles();

		case XED_ICLASS_JP:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JP;
			return getJPCycles();

		case XED_ICLASS_JO:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JO;
			return getJOCycles();
		
		case XED_ICLASS_JS:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JS;
			return getJSCycles();

		case XED_ICLASS_JNZ:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JNZ;
			return getJNZCycles();

		case XED_ICLASS_JNL:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JNL;
			return getJNLCycles();

		case XED_ICLASS_JNLE:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JNLE;
			return getJNLECycles();

		case XED_ICLASS_JNB:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JNB;
			return getJNBCycles();

		case XED_ICLASS_JNBE:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JNBE;
			return getJNBECycles();

		case XED_ICLASS_JNP:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JNP;
			return getJNPCycles();

		case XED_ICLASS_JNS:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JNS;
			return getJNSCycles();

		case XED_ICLASS_LOOP:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = LOOP;
			return getLOOPCycles();
		
		case XED_ICLASS_LOOPE:// = LOOPZ
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = LOOPZ;
			return getLOOPZCycles();

		case XED_ICLASS_LOOPNE:// = LOOPNZ
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = LOOPNZ;
			return getLOOPNZCycles();

		case XED_ICLASS_JCXZ:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = JCXZ;
			return getJCXZCycles();

		case XED_ICLASS_INT:
			cpu.biu.startControlTransferInstruction();
			cpu.eu.instructionFunction = INT;
			return getINTCycles();

		case XED_ICLASS_INTO:
			cpu.eu.instructionFunction = INTO;
			return getINTOCycles();

		case XED_ICLASS_IRET:
			cpu.eu.instructionFunction = IRET;
			return getIRETCycles();

		case XED_ICLASS_CLC:
			cpu.eu.instructionFunction = CLC;
			return getCLCCycles();

		case XED_ICLASS_CMC:
			cpu.eu.instructionFunction = CMC;
			return getCMCCycles();

		case XED_ICLASS_STC:
			cpu.eu.instructionFunction = STC;
			return getSTCCycles();

		case XED_ICLASS_CLD:
			cpu.eu.instructionFunction = CLD;
			return getCLDCycles();

		case XED_ICLASS_STD:
			cpu.eu.instructionFunction = STD;
			return getSTDCycles();

		case XED_ICLASS_CLI:
			cpu.eu.instructionFunction = CLI;
			return getCLICycles();

		case XED_ICLASS_STI:
			cpu.eu.instructionFunction = STI;
			return getSTICycles();

		case XED_ICLASS_HLT:
			cpu.eu.instructionFunction = HLT;
			return getHLTCycles();

		case XED_ICLASS_NOP:
			cpu.eu.instructionFunction = NOP;
			return getNOPCycles();

	default:
		return 1;
	}
}

static bool EUWaitSuccessfullDecodeClock(void)
{
	xed_decoded_inst_zero_keep_mode(&cpu.eu.decodedInst);
	const xed_error_enum_t DECODE_STATUS = xed_decode(&cpu.eu.decodedInst,cpu.biu.instructionBufferQueue.data(),cpu.biu.instructionBufferQueuePos);

	if (DECODE_STATUS == XED_ERROR_NONE)
	{
		INSTRUCTION_CLOCK_LEFT = prepareInstructionExecution();
		cpu.biu.instructionBufferQueuePop(xed_decoded_inst_get_length(&cpu.eu.decodedInst));
		cpu.instructionExecutedCount += 1;
		cpu.eu.nextClockFunction = EUExecInstructionClock;
		#ifdef SEE_CURRENT_INST
			printCurrentInstruction();
		#endif
	}

	return false;
}

EU::EU(): clock(EUWaitSuccessfullDecodeClock), nextClockFunction(EUWaitSuccessfullDecodeClock) {}

void EU::push (const uint16_t data)
{ cpu.sp -= 2; cpu.biu.writeWord(cpu.genAddress(cpu.ss,cpu.sp), data); }

uint16_t EU::pop (void)
{
	const uint16_t ret = cpu.biu.readWord(cpu.genAddress(cpu.ss,cpu.sp)); cpu.sp += 2;
	return ret;
}

void EU::farCall (const uint16_t seg, const uint16_t offset)
{
	push(cpu.cs);
	push(cpu.ip);

	cpu.cs = seg;
	cpu.ip = offset;
}

void EU::farRet (void)
{
	cpu.ip = pop();
	cpu.cs = pop();
}

/*bool EU::clock()
{
	static unsigned int clockCountDown = 0;

	

	if (clockCountDown > 0)
	{
		#if defined(SEE_CURRENT_INST) || defined(SEE_ALL)
			printCurrentInstruction();
			printf(" clock left: %d\n",clockCountDown);
		#endif
		clockCountDown -= 1;
	}

	if (clockCountDown > 0)
		return false;

	if (newFetchAddress)
	{
		cpu.biu.endControlTransferInstruction(cpu.cs,cpu.ip);
		newFetchAddress = false;
	}

	#if defined(SEE_CURRENT_INST) || defined(SEE_ALL)
	if (cpu.instructionExecutedCount != 0)
	{
		printCurrentInstruction(); printf(" (%d)\n",cpu.instructionExecutedCount);
		printRegisters();
		printFlags();
	}
	#endif

	

	return instructionDecoded;
}*/

/**
 * This function compute the effective address for a memory operand and add the corresponding number of 
 * instructions cycles. The instruction cycles are picked up from https://zsmith.co/intel.php#ea :
 * 1.  disp: mod = 0b00 and rm = 0b110								+6
 * 2.  (BX,BP,SI,DI): mod = 0b00 and rm != 0b110 and rm = 0b1xx		+5
 * 3.  disp + (BX,BP,SI,DI): mod = 0b10 and rm = 0b1xx				+9
 * 4.1 (BP+DI, bx+SI): mod = 0b00 and rm = 0b01x					+7
 * 4.2 (BP+SI, bx+DI): mod = 0b00 and rm = 0b00x					+8
 * 5.1 disp + (BP+DI, bx+SI) +-> same as precedet with mod = 0b10	+11
 * 5.2 disp + (BP+SI, bx+DI) +										+12
 *
 * word operands at odd addresses	+4
 * segment override					+2
 */
unsigned int EU::getEAComputationClockCount()
{
	const unsigned int modrm = xed_decoded_inst_get_modrm(&decodedInst);
	const unsigned int mod = (modrm & 0b11000000) >> 6;
	const unsigned int rm = modrm & 0b111;

	unsigned int clockNeeded = 0;

	if (mod == 0b00)
	{
		//1. disp: mod == 0b00 and rm 0b110
		if (rm == 0b110)
			clockNeeded += 6;
		else
		{
			//2. (base,index) mod = 0b00 and rm = 0b1xx and rm != 0b110
			if (rm & 0b100)
				clockNeeded += 5;
			//4.1/4.2 base + index mod = 0b00 and rm = 0b01x/0b00x
			else
				clockNeeded += (rm & 0b10) ? 7 : 8;
		}
	}
	//mod = 0b10
	else
	{
		//3. disp + (base,index): mod = 0b10 rm = 0b1xx
		if (rm & 0b100)
			clockNeeded += 9;
		//5.1/5.2 base + index + disp: mod = 0b10 rm = 0b01x/0b00x
		else
			clockNeeded += (rm & 0b10) ? 11 : 12;
	}

	cpu.eu.EAAddress = cpu.genAddress(xed_decoded_inst_get_seg_reg(&decodedInst,0), xed_decoded_inst_get_memory_displacement(&decodedInst,0));
	
	if (xed_operand_values_has_segment_prefix(&decodedInst))
		clockNeeded += 2;
	
	return clockNeeded;
}
