#include "eu.hpp"
#include "arch.hpp"

#include "instructions.hpp"

using namespace E5150::I8086;
static bool EUWaitSuccessfullDecodeClock(void);

static bool (*nextClockFunction)(void) = nullptr;
static void (*instructionFunction)(void) = nullptr;
static unsigned int (*repInstructionGetNextClockCount)(const unsigned int) = nullptr;
static unsigned int INSTRUCTION_CLOCK_LEFT = 0;
static unsigned int REP_COUNT = 0;

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
		printf(" (iform: '%s') ", xed_iform_enum_t2str(xed_decoded_inst_get_iform_enum(&cpu.eu.decodedInst)));

	printf(" (%d)\n",cpu.instructionExecutedCount);
	#endif
}

static bool EUExecInstructionClock(void)
{
	#if defined(SEE_ALL) || defined(SEE_CURRENT_INST)
		printCurrentInstruction();
	printf(" clock left : %d\n",INSTRUCTION_CLOCK_LEFT);
	#endif
	if (INSTRUCTION_CLOCK_LEFT == 0)
	{
		instructionFunction();

		nextClockFunction = EUWaitSuccessfullDecodeClock;
		return true;
	}

	INSTRUCTION_CLOCK_LEFT -= 1;
	return false;
}

static bool EUExecRepInstructionClock(void)
{
	#if defined(SEE_ALL) || defined(SEE_CURRENT_INST)
		printCurrentInstruction();   printf(" clock left : %d\n",INSTRUCTION_CLOCK_LEFT);
	#endif

	if (INSTRUCTION_CLOCK_LEFT == 0)
	{
		++REP_COUNT;
		instructionFunction();
		
		if (cpu.eu.repInstructionFinished)
		{
			nextClockFunction = EUWaitSuccessfullDecodeClock;
			cpu.eu.repInstructionFinished = false;
			return true;
		}
		else
		{
			INSTRUCTION_CLOCK_LEFT = repInstructionGetNextClockCount(REP_COUNT);
		}
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
	cpu.biu.requestMemoryByte(memoryByteAccess);
	cpu.eu.instructionLength = xed_decoded_inst_get_length(&cpu.eu.decodedInst);

	switch (xed_decoded_inst_get_iclass(&cpu.eu.decodedInst))
	{
		case XED_ICLASS_MOV:
			instructionFunction = MOV;
			return getMOVCycles();

		case XED_ICLASS_PUSH:
			instructionFunction = PUSH;
			return getPUSHCycles();

		case XED_ICLASS_POP:
			instructionFunction = POP;
			return getPOPCycles();

		case XED_ICLASS_XCHG:
			instructionFunction = XCHG;
			return getXCHGCycles();

		case XED_ICLASS_IN:
			instructionFunction = IN;
			return getINCycles();

		case XED_ICLASS_OUT:
			instructionFunction = OUT;
			return getOUTCycles();

		case XED_ICLASS_XLAT:
			instructionFunction = XLAT;
			return getXLATCycles();

		case XED_ICLASS_LEA:
			instructionFunction = LEA;
			return getLEACycles();

		case XED_ICLASS_LDS:
			instructionFunction = LDS;
			return getLDSCycles();

		case XED_ICLASS_LES:
			instructionFunction = LES;
			return getLESCycles();

		case XED_ICLASS_LAHF:
			instructionFunction = LAHF;
			return getLAHFCycles();

		case XED_ICLASS_SAHF:
			instructionFunction = SAHF;
			return getSAHFCycles();

		case XED_ICLASS_PUSHF:
			instructionFunction = PUSHF;
			return getPUSHFCycles();

		case XED_ICLASS_POPF:
			instructionFunction = POPF;
			return getPOPFCycles();

		case XED_ICLASS_ADD:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.withCarry = false;
			instructionFunction = ADD;
			return getADDCycles();

		case XED_ICLASS_ADC:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.withCarry = true;
			instructionFunction = ADD;
			return getADDCycles();

		case XED_ICLASS_INC:
			instructionFunction = INC;
			return getINCCycles();

		case XED_ICLASS_AAA:
			instructionFunction = AAA;
			return getAAACycles();

		case XED_ICLASS_DAA:
			instructionFunction = DAA;
			return getDAACycles();

		case XED_ICLASS_SUB:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.withCarry = false;
			instructionFunction = SUB;
			return getSUBCycles();

		case XED_ICLASS_SBB:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.withCarry = true;
			instructionFunction = SUB;
			return getSUBCycles();

		case XED_ICLASS_DEC:
			instructionFunction = DEC;
			return getDECCycles();

		case XED_ICLASS_NEG:
			instructionFunction = NEG;
			return getNEGCycles();

		case XED_ICLASS_CMP:
			instructionFunction = CMP;
			return getCMPCycles();

		case XED_ICLASS_AAS:
			instructionFunction = AAS;
			return getAASCycles();

		case XED_ICLASS_DAS:
			instructionFunction = DAS;
			return getDASCycles();

		case XED_ICLASS_MUL:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.isSigned = false;
			instructionFunction = MUL;
			return getMULCycles();

		case XED_ICLASS_IMUL:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.isSigned = true;
			instructionFunction = MUL;
			return getIMULCycles();

		case XED_ICLASS_DIV:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.isSigned = false;
			instructionFunction = DIV;
			return getDIVCycles();

		case XED_ICLASS_IDIV:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.isSigned = true;
			instructionFunction = DIV;
			return getIDIVCycles();

		case XED_ICLASS_AAD:
			instructionFunction = AAD;
			return getAADCycles();

		case XED_ICLASS_CBW:
			instructionFunction = CBW;
			return getCBWCycles();

		case XED_ICLASS_CWD:
			instructionFunction = CWD;
			return getCWDCycles();

		case XED_ICLASS_NOT:
			instructionFunction = NOT;
			return getNOTCycles();

		case XED_ICLASS_SHL:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.setDirectionIsLeft();
			instructionFunction = SHIFT;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_SHR:
			instructionFunction = SHIFT;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_SAR:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.setInstructionIsArithmetic();
			instructionFunction = SHIFT;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_ROL:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.setDirectionIsLeft();
			instructionFunction = ROTATE;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_ROR:
			instructionFunction = ROTATE;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_RCL:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.setRotationWithCarry();
			cpu.eu.instructionExtraData.setDirectionIsLeft();
			instructionFunction = ROTATE;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_RCR:
			cpu.eu.instructionExtraData.isSigned = 0;
			cpu.eu.instructionExtraData.setRotationWithCarry();
			instructionFunction = ROTATE;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_AND:
			instructionFunction = AND;
			return getANDCycles();

		case XED_ICLASS_TEST:
			instructionFunction = TEST;
			return getTESTCycles();

		case XED_ICLASS_OR:
			instructionFunction = OR;
			return getORCycles();

		case XED_ICLASS_XOR:
			instructionFunction = XOR;
			return getXORCycles();
		
		case XED_ICLASS_MOVSB:
		case XED_ICLASS_MOVSW:
			instructionFunction = MOVS;
			return getMOVSCycles();

		case XED_ICLASS_REP_MOVSB:
		case XED_ICLASS_REP_MOVSW:
			REP_COUNT = 0;
			instructionFunction = REP_MOVS;
			repInstructionGetNextClockCount = getREP_MOVSCycles;
			return getREP_MOVSCycles(REP_COUNT);

		case XED_ICLASS_CMPSB:
		case XED_ICLASS_CMPSW:
			instructionFunction = CMPS;
			return getCMPSCycles();

		case XED_ICLASS_REPE_CMPSB:
		case XED_ICLASS_REPNE_CMPSB:
		case XED_ICLASS_REPE_CMPSW:
		case XED_ICLASS_REPNE_CMPSW:
			REP_COUNT = 0;
			instructionFunction = REP_CMPS;
			repInstructionGetNextClockCount = getREP_CMPSCycles;
			return getREP_CMPSCycles(REP_COUNT);

		case XED_ICLASS_SCASB:
		case XED_ICLASS_SCASW:
			instructionFunction = SCAS;
			return getSCASCycles();

		case XED_ICLASS_REPE_SCASB:
		case XED_ICLASS_REPNE_SCASB:
		case XED_ICLASS_REPE_SCASW:
		case XED_ICLASS_REPNE_SCASW:
			REP_COUNT = 0;
			instructionFunction = REP_SCAS;
			repInstructionGetNextClockCount = getREP_SCASCycles;
			return getSCASCycles();

		case XED_ICLASS_LODSB:
		case XED_ICLASS_LODSW:
			instructionFunction = LODS;
			return getLODSCycles();

		case XED_ICLASS_REP_LODSB:
		case XED_ICLASS_REP_LODSW:
			REP_COUNT = 0;
			instructionFunction = REP_LODS;
			repInstructionGetNextClockCount = getREP_LODSCycles;
			return getREP_LODSCycles(REP_COUNT);

		case XED_ICLASS_STOSB:
		case XED_ICLASS_STOSW:
			instructionFunction = STOS;
			return getSTOSCycles();

		case XED_ICLASS_REP_STOSB:
		case XED_ICLASS_REP_STOSW:
			REP_COUNT = 0;
			instructionFunction = REP_STOS;
			repInstructionGetNextClockCount = getREP_STOSCycles;
			return getREP_STOSCycles(REP_COUNT);

		case XED_ICLASS_CALL_NEAR:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = CALL_NEAR;
			return getCALLCycles();

		case XED_ICLASS_CALL_FAR:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = CALL_FAR;
			return getCALL_FARCycles();

		case XED_ICLASS_JMP:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JMP_NEAR;
			return getJMPCycles();

		case XED_ICLASS_JMP_FAR:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JMP_FAR;
			return getJMP_FARCycles();

		case XED_ICLASS_RET_NEAR:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = RET_NEAR;
			return getRETCycles();

		case XED_ICLASS_RET_FAR:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = RET_FAR;
			return getRET_FARCycles();

		case XED_ICLASS_JZ:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JZ;
			return getJXXCycles(cpu.getFlagStatus(CPU::ZERRO));

		case XED_ICLASS_JL:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JL;
			return getJXXCycles(cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER));

		case XED_ICLASS_JLE:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JLE;
			return getJXXCycles(cpu.getFlagStatus(CPU::ZERRO) || (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER)));

		case XED_ICLASS_JB:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JB;
			return getJXXCycles(CPU::CARRY);

		case XED_ICLASS_JBE:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JBE;
			return getJXXCycles(CPU::CARRY || CPU::ZERRO);

		case XED_ICLASS_JP:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JP;
			return getJXXCycles(CPU::PARRITY);

		case XED_ICLASS_JO:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JO;
			return getJXXCycles(cpu.getFlagStatus(CPU::OVER));
		
		case XED_ICLASS_JS:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JS;
			return getJXXCycles(cpu.getFlagStatus(CPU::SIGN));

		case XED_ICLASS_JNZ:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JNZ;
			return getJXXCycles(!cpu.getFlagStatus(CPU::ZERRO));

		case XED_ICLASS_JNL:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JNL;
			return getJXXCycles(cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER));

		case XED_ICLASS_JNLE:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JNLE;
			return getJXXCycles(!cpu.getFlagStatus(CPU::ZERRO) && (cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER)));

		case XED_ICLASS_JNB:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JNB;
			return getJXXCycles(!cpu.getFlagStatus(CPU::CARRY));

		case XED_ICLASS_JNBE:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JNBE;
			return getJXXCycles(!cpu.getFlagStatus(CPU::CARRY) && !cpu.getFlagStatus(CPU::ZERRO));

		case XED_ICLASS_JNP:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JNP;
			return getJXXCycles(!cpu.getFlagStatus(CPU::PARRITY));

		case XED_ICLASS_JNS:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JNS;
			return getJXXCycles(!cpu.getFlagStatus(CPU::SIGN));

		case XED_ICLASS_LOOP:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = LOOP;
			return getLOOPCycles();
		
		case XED_ICLASS_LOOPE:// = LOOPZ
			cpu.biu.startControlTransferInstruction();
			instructionFunction = LOOPZ;
			return getLOOPZCycles();

		case XED_ICLASS_LOOPNE:// = LOOPNZ
			cpu.biu.startControlTransferInstruction();
			instructionFunction = LOOPNZ;
			return getLOOPNZCycles();

		case XED_ICLASS_JCXZ:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = JCXZ;
			return getJCXZCycles();

		case XED_ICLASS_INT:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = INT;
			return getINTCycles();
		
		case XED_ICLASS_INT3:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = INT3;
			return getINTCycles();

		case XED_ICLASS_INTO:
			cpu.biu.startControlTransferInstruction();
			instructionFunction = INTO;
			return getINTCycles();

		case XED_ICLASS_IRET:
			instructionFunction = IRET;
			return getIRETCycles();

		case XED_ICLASS_CLC:
			instructionFunction = CLC;
			return getCLCCycles();

		case XED_ICLASS_CMC:
			instructionFunction = CMC;
			return getCMCCycles();

		case XED_ICLASS_STC:
			instructionFunction = STC;
			return getSTCCycles();

		case XED_ICLASS_CLD:
			instructionFunction = CLD;
			return getCLDCycles();

		case XED_ICLASS_STD:
			instructionFunction = STD;
			return getSTDCycles();

		case XED_ICLASS_CLI:
			instructionFunction = CLI;
			return getCLICycles();

		case XED_ICLASS_STI:
			instructionFunction = STI;
			return getSTICycles();

		case XED_ICLASS_HLT:
			instructionFunction = HLT;
			return getHLTCycles();

		case XED_ICLASS_NOP:
			instructionFunction = NOP;
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
		#if defined(SEE_ALL) || defined(SEE_CURRENT_INST)
			printCurrentInstruction();
		#endif
		INSTRUCTION_CLOCK_LEFT = prepareInstructionExecution();
		nextClockFunction = EUExecInstructionClock;
		#if defined(SEE_ALL) || defined(SEE_CURRENT_INST)
			printf("Clock cycles: %d\n",INSTRUCTION_CLOCK_LEFT);
		#endif
	}

	return false;
}

EU::EU(): clock(EUWaitSuccessfullDecodeClock) { nextClockFunction = EUWaitSuccessfullDecodeClock; }

void EU::updateClockFunction() { clock = nextClockFunction; }

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
unsigned int EU::computeEAAndGetComputationClockCount()
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
