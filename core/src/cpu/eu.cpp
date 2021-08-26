#include "eu.hpp"
#include "arch.hpp"

#include "instructions.hpp"

using namespace E5150::I8086;

EU::EU(): newFetchAddress(false) {}

//TODO: How to handle REP, LOCK, WAIT and ESC
static unsigned int execInstructionAndGetClockCycles(void)
{
	//At the end of the instructions that access memory there is w bit = 0 for byte operand and 1 one for word operands.
	//If this bit = 0 there is 1 memory access and if it = 1, 2 memory access
	const unsigned int memoryByteAccess = xed_decoded_inst_number_of_memory_operands(&cpu.eu.decodedInst) * ((cpu.biu.instructionBufferQueue[0] & 0b1) + 1);
	switch (xed_decoded_inst_get_iclass(&cpu.eu.decodedInst))
	{
		case XED_ICLASS_MOV:
			MOV();
			return getMOVCycles();

		case XED_ICLASS_PUSH:
			PUSH();
			return getPUSHCycles();

		case XED_ICLASS_POP:
			POP();
			return getPOPCycles();

		case XED_ICLASS_XCHG:
			XCHG();
			return getXCHGCycles();

		case XED_ICLASS_IN:
			IN();
			return getINCycles();

		case XED_ICLASS_OUT:
			OUT();
			return getOUTCycles();

		case XED_ICLASS_XLAT:
			XLAT();
			return getXLATCycles();

		case XED_ICLASS_LEA:
			LEA();
			return getLEACycles();

		case XED_ICLASS_LDS:
			LDS();
			return getLDSCycles();

		case XED_ICLASS_LES:
			LES();
			return getLESCycles();

		case XED_ICLASS_LAHF:
			LAHF();
			return getLAHFCycles();

		case XED_ICLASS_SAHF:
			SAHF();
			return getSAHFCycles();

		case XED_ICLASS_PUSHF:
			PUSHF();
			return getPUSHFCycles();

		case XED_ICLASS_POPF:
			POPF();
			return getPOPFCycles();

		case XED_ICLASS_ADD:
			ADD();
			return getADDCycles();

		case XED_ICLASS_ADC:
			ADD(true);
			return getADDCycles();

		case XED_ICLASS_INC:
			INC();
			return getINCCycles();

		case XED_ICLASS_AAA:
			AAA();
			return getAAACycles();

		case XED_ICLASS_DAA:
			DAA();
			return getDAACycles();

		case XED_ICLASS_SUB:
			SUB();
			return getSUBCycles();

		case XED_ICLASS_SBB:
			SUB(true);
			return getSUBCycles();

		case XED_ICLASS_DEC:
			DEC();
			return getDECCycles();

		case XED_ICLASS_NEG:
			NEG();
			return getNEGCycles();

		case XED_ICLASS_CMP:
			CMP();
			return getCMPCycles();

		case XED_ICLASS_AAS:
			AAS();
			return getAASCycles();

		case XED_ICLASS_DAS:
			DAS();
			return getDASCycles();

		case XED_ICLASS_MUL:
			MUL();
			return getMULCycles();

		case XED_ICLASS_IMUL:
			MUL(true);
			return getIMULCycles();

		case XED_ICLASS_DIV:
			DIV();
			return getDIVCycles();

		case XED_ICLASS_IDIV:
			DIV(true);
			return getIDIVCycles();

		case XED_ICLASS_AAD:
			AAD();
			return getAADCycles();

		case XED_ICLASS_CBW:
			CBW();
			return getCBWCycles();

		case XED_ICLASS_CWD:
			CWD();
			return getCWDCycles();

		case XED_ICLASS_NOT:
			NOT();
			return getNOTCycles();

		case XED_ICLASS_SHL:
			SHL();
			return getSHLCycles();

		case XED_ICLASS_SHR:
			SHR();
			return getSHRCycles();

		case XED_ICLASS_SAR:
			SAR();
			return getSARCycles();

		case XED_ICLASS_ROL:
			ROL();
			return getROLCycles();

		case XED_ICLASS_ROR:
			ROR();
			return getRORCycles();

		case XED_ICLASS_RCL:
			RCL();
			return getRCLCycles();

		case XED_ICLASS_RCR:
			RCR();
			return getRCRCycles();

		case XED_ICLASS_AND:
			AND();
			return getANDCycles();

		case XED_ICLASS_TEST:
			TEST();
			return getTESTCycles();

		case XED_ICLASS_OR:
			OR();
			return getORCycles();

		case XED_ICLASS_XOR:
			XOR();
			return getXORCycles();

		case XED_ICLASS_MOVSB:
		case XED_ICLASS_MOVSW:
			MOVS();
			return getMOVSCycles();

		case XED_ICLASS_CMPSB:
		case XED_ICLASS_CMPSW:
			CMPS();
			return getCMPSCycles();

		case XED_ICLASS_SCASB:
		case XED_ICLASS_SCASW:
			SCAS();
			return getSCASCycles();

		case XED_ICLASS_LODSB:
		case XED_ICLASS_LODSW:
			LODS();
			return getLODSCycles();

		case XED_ICLASS_STOSB:
		case XED_ICLASS_STOSW:
			STOS();
			return getSTOSCycles();

		case XED_ICLASS_CALL_NEAR:
			CALL_NEAR();
			return getCALL_NEARCycles();

		case XED_ICLASS_CALL_FAR:
			CALL_FAR();
			return getCALL_FARCycles();

		case XED_ICLASS_JMP:
			JMP_NEAR();
			return getJMP_NEARCycles();

		case XED_ICLASS_JMP_FAR:
			JMP_FAR();
			return getJMP_FARCycles();

		case XED_ICLASS_RET_NEAR:
			RET_NEAR();
			return getRET_NEARCycles();

		case XED_ICLASS_RET_FAR:
			RET_FAR();
			return getRET_FARCycles();

		case XED_ICLASS_JZ:
			JZ();
			return getJZCycles();

		case XED_ICLASS_JL:
			JL();
			return getJLCycles();

		case XED_ICLASS_JLE:
			JLE();
			return getJLECycles();

		case XED_ICLASS_JB:
			JB();
			return getJBCycles();

		case XED_ICLASS_JBE:
			JBE();
			return getJBECycles();

		case XED_ICLASS_JP:
			JP();
			return getJPCycles();

		case XED_ICLASS_JO:
			JO();
			return getJOCycles();
		
		case XED_ICLASS_JS:
			JS();
			return getJSCycles();

		case XED_ICLASS_JNZ:
			JNZ();
			return getJNZCycles();

		case XED_ICLASS_JNL:
			JNL();
			return getJNLCycles();

		case XED_ICLASS_JNLE:
			JNLE();
			return getJNLECycles();

		case XED_ICLASS_JNB:
			JNB();
			return getJNBCycles();

		case XED_ICLASS_JNBE:
			JNBE();
			return getJNBECycles();

		case XED_ICLASS_JNP:
			JNP();
			return getJNPCycles();

		case XED_ICLASS_JNS:
			JNS();
			return getJNSCycles();

		case XED_ICLASS_LOOP:
			LOOP();
			return getLOOPCycles();
		
		case XED_ICLASS_LOOPE:// = LOOPZ
			LOOPZ();
			return getLOOPZCycles();

		case XED_ICLASS_LOOPNE:// = LOOPNZ
			LOOPNZ();
			return getLOOPNZCycles();

		case XED_ICLASS_JCXZ:
			JCXZ();
			return getJCXZCycles();

		case XED_ICLASS_INT:
			INT();
			return getINTCycles();

		case XED_ICLASS_INTO:
			INTO();
			return getINTOCycles();

		case XED_ICLASS_IRET:
			IRET();
			return getIRETCycles();

		case XED_ICLASS_CLC:
			CLC();
			return getCLCCycles();

		case XED_ICLASS_CMC:
			CMC();
			return getCMCCycles();

		case XED_ICLASS_STC:
			STC();
			return getSTCCycles();

		case XED_ICLASS_CLD:
			CLD();
			return getCLDCycles();

		case XED_ICLASS_STD:
			STD();
			return getSTDCycles();

		case XED_ICLASS_CLI:
			CLI();
			return getCLICycles();

		case XED_ICLASS_STI:
			STI();
			return getSTICycles();

		case XED_ICLASS_HLT:
			HLT();
			return getHLTCycles();

		case XED_ICLASS_NOP:
			NOP();
			return getNOPCycles();

	default:
		return 1;
	}
}

void EU::push (const uint16_t data)
{ cpu.sp -= 2; cpu.biu.EURequestWriteWord(cpu.genAddress(cpu.ss,cpu.sp), data); }

uint16_t EU::pop (void)
{
	const uint16_t ret = cpu.biu.EURequestReadWord(cpu.genAddress(cpu.ss,cpu.sp)); cpu.sp += 2;
	return ret;
}

void EU::farCall (const uint16_t seg, const uint16_t offset)
{
	push(cpu.cs);
	push(cpu.ip);

	newCS = seg;
	newIP = offset;
	newFetchAddress = true;
}

void EU::farRet (void)
{
	newIP = pop();
	newCS = pop();
	newFetchAddress = true;
}

static void printCurrentInstruction(void)
{
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

		if (op_vis == XED_OPVIS_EXPLICIT)
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

bool EU::clock()
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
		cpu.biu.endControlTransferInstruction(newCS,newIP);
		newFetchAddress = false;
	}

	xed_decoded_inst_zero_keep_mode(&decodedInst);
	const xed_error_enum_t DECODE_STATUS = xed_decode(&decodedInst,cpu.biu.instructionBufferQueue.data(),cpu.biu.instructionBufferQueuePos);

	if (DECODE_STATUS == xed_error_enum_t::XED_ERROR_NONE)
	{
		clockCountDown = execInstructionAndGetClockCycles();
		cpu.biu.instructionBufferQueuePop(xed_decoded_inst_get_length(&decodedInst));
		cpu.instructionExecuted += 1;

		#if defined(SEE_CURRENT_INST) || defined(SEE_ALL)
			printCurrentInstruction(); printf(" (%d)\n",cpu.instructionExecuted);
			printRegisters();
			printFlags();
		#endif
	}

	return DECODE_STATUS == xed_error_enum_t::XED_ERROR_NONE;
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
