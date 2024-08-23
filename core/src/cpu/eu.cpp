#include "core/eu.hpp"
#include "core/arch.hpp"

#include "core/instructions.hpp"

using namespace E5150::I8086;
static bool EUWaitSuccessfullDecodeClock(void);

static void (*instructionFunction)(void) = nullptr;
static unsigned int (*repInstructionGetNextClockCount)(const unsigned int) = nullptr;
static EU::InternalInfos EUWorkingState;

static void doNothing(void) { return; }

static bool EUExecInterruptServiceProcedureClock(void)
{
	EUWorkingState.INSTRUCTION_CLOCK_LEFT -= 1;

	#ifdef STOP_AT_CLOCK
		printf("EU: INTERRUPT SERVICE PROCEDURE: clock left: %d\n", workingState.INSTRUCTION_CLOCK_LEFT);
	#endif

	if (EUWorkingState.INSTRUCTION_CLOCK_LEFT == 0)
	{
		if (cpu.interruptSequence())
		{
			EMULATION_INFO_LOG<EMULATION_MAX_LOG_LEVEL>("[EU] New interrupt detected while servicing an interrupt");
			cpu.handleInterrupts();
		}
		else
		{
			cpu.biu.endInterruptDataSaveSequence();
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::DECODE;
		}
	}

	return 0;
}

static unsigned int EUExecInstructionClock(void)
{
	#ifdef STOP_AT_CLOCK
		printCurrentInstruction();
		printf(" clock left : %d\n",workingState.INSTRUCTION_CLOCK_LEFT);
	#endif
	if (EUWorkingState.INSTRUCTION_CLOCK_LEFT == 0)
	{
		instructionFunction();
		EUWorkingState.EUWorkingMode = EU::WORKING_MODE::DECODE;
		return E5150::I8086::EU::STATUS_INSTRUCTION_EXECUTED;
	}

	EUWorkingState.INSTRUCTION_CLOCK_LEFT -= 1;
	return 0;
}

static bool EUExecRepInstructionClock(void)
{
	#if defined(STOP_AT_CLOCK)
		printCurrentInstruction();   printf(" clock left : %d\n",workingState.INSTRUCTION_CLOCK_LEFT);
	#endif

	if (EUWorkingState.INSTRUCTION_CLOCK_LEFT == 0)
	{
		++EUWorkingState.REP_COUNT;
		instructionFunction();
		
		if (cpu.eu.repInstructionFinished)
		{
			cpu.eu.repInstructionFinished = false;
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::DECODE;
			return E5150::I8086::EU::STATUS_INSTRUCTION_EXECUTED;
		}
		else
		{
			EUWorkingState.INSTRUCTION_CLOCK_LEFT = repInstructionGetNextClockCount(EUWorkingState.REP_COUNT);
		}
	}

	EUWorkingState.INSTRUCTION_CLOCK_LEFT -= 1;
	return 0;
}

//TODO: How to hanregs.dle LOCK, WAIT and ESC
static unsigned int prepareInstructionExecution(void)
{
	//At the end of the opcode of instructions that access memory, there is w bit = 0 for byte operand and 1 one for word operands.
	//If this bit = 0 there is 1 memory access and if it = 1, 2 memory accesses
	const unsigned int nPrefix = xed_decoded_inst_get_nprefixes(&cpu->decodedInst);
	cpu.eu.operandSizeWord = cpu.biu.instructionBufferQueue[nPrefix] & 0b1;
	const unsigned int memoryByteAccess = xed_decoded_inst_number_of_memory_operands(&cpu->decodedInst) * (cpu.eu.operandSizeWord + 1);
	cpu.biu.requestMemoryByte(memoryByteAccess);
	cpu.eu.instructionLength = xed_decoded_inst_get_length(&cpu->decodedInst);
	EUWorkingState.CURRENT_INSTRUCTION_CS = cpu.regs.cs;
	EUWorkingState.CURRENT_INSTRUCTION_IP = cpu.regs.ip;
	cpu.biu.IPToNextInstruction(cpu.eu.instructionLength);
	EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_INSTRUCTION;

	switch (xed_decoded_inst_get_iclass(&cpu->decodedInst))
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
			instructionFunction = _IN;
			return getINCycles();

		case XED_ICLASS_OUT:
			instructionFunction = _OUT;
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
			cpu.eu.instructionExtraData.clearData();
			instructionFunction = ADD;
			return getADDCycles();

		case XED_ICLASS_ADC:
			cpu.eu.instructionExtraData.clearData();
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
			cpu.eu.instructionExtraData.clearData();
			instructionFunction = SUB;
			return getSUBCycles();

		case XED_ICLASS_SBB:
			cpu.eu.instructionExtraData.clearData();
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
			cpu.eu.instructionExtraData.clearData();
			instructionFunction = MUL;
			return getMULCycles();

		case XED_ICLASS_IMUL:
			cpu.eu.instructionExtraData.isSigned = true;
			instructionFunction = MUL;
			return getIMULCycles();

		case XED_ICLASS_DIV:
			cpu.eu.instructionExtraData.clearData();
			instructionFunction = DIV;
			return getDIVCycles();

		case XED_ICLASS_IDIV:
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
			cpu.eu.instructionExtraData.clearData();
			cpu.eu.instructionExtraData.setDirectionIsLeft();
			instructionFunction = SHIFT;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_SHR:
			cpu.eu.instructionExtraData.clearData();
			instructionFunction = SHIFT;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_SAR:
			cpu.eu.instructionExtraData.clearData();
			cpu.eu.instructionExtraData.setInstructionIsArithmetic();
			instructionFunction = SHIFT;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_ROL:
			cpu.eu.instructionExtraData.clearData();
			cpu.eu.instructionExtraData.setDirectionIsLeft();
			instructionFunction = ROTATE;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_ROR:
			instructionFunction = ROTATE;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_RCL:
			cpu.eu.instructionExtraData.clearData();
			cpu.eu.instructionExtraData.setRotationWithCarry();
			cpu.eu.instructionExtraData.setDirectionIsLeft();
			instructionFunction = ROTATE;
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_RCR:
			cpu.eu.instructionExtraData.clearData();
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
			EUWorkingState.REP_COUNT = 0;
			instructionFunction = REP_MOVS;
			repInstructionGetNextClockCount = getREP_MOVSCycles;
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_REP_INSTRUCTION;
			return getREP_MOVSCycles(EUWorkingState.REP_COUNT);

		case XED_ICLASS_CMPSB:
		case XED_ICLASS_CMPSW:
			instructionFunction = CMPS;
			return getCMPSCycles();

		case XED_ICLASS_REPE_CMPSB:
		case XED_ICLASS_REPNE_CMPSB:
		case XED_ICLASS_REPE_CMPSW:
		case XED_ICLASS_REPNE_CMPSW:
			EUWorkingState.REP_COUNT = 0;
			instructionFunction = REP_CMPS;
			repInstructionGetNextClockCount = getREP_CMPSCycles;
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_REP_INSTRUCTION;
			return getREP_CMPSCycles(EUWorkingState.REP_COUNT);

		case XED_ICLASS_SCASB:
		case XED_ICLASS_SCASW:
			instructionFunction = SCAS;
			return getSCASCycles();

		case XED_ICLASS_REPE_SCASB:
		case XED_ICLASS_REPNE_SCASB:
		case XED_ICLASS_REPE_SCASW:
		case XED_ICLASS_REPNE_SCASW:
			EUWorkingState.REP_COUNT = 0;
			instructionFunction = REP_SCAS;
			repInstructionGetNextClockCount = getREP_SCASCycles;
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_REP_INSTRUCTION;
			return getREP_SCASCycles(EUWorkingState.REP_COUNT);

		case XED_ICLASS_LODSB:
		case XED_ICLASS_LODSW:
			instructionFunction = LODS;
			return getLODSCycles();

		case XED_ICLASS_REP_LODSB:
		case XED_ICLASS_REP_LODSW:
			EUWorkingState.REP_COUNT = 0;
			instructionFunction = REP_LODS;
			repInstructionGetNextClockCount = getREP_LODSCycles;
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_REP_INSTRUCTION;
			return getREP_LODSCycles(EUWorkingState.REP_COUNT);

		case XED_ICLASS_STOSB:
		case XED_ICLASS_STOSW:
			instructionFunction = STOS;
			return getSTOSCycles();

		case XED_ICLASS_REP_STOSB:
		case XED_ICLASS_REP_STOSW:
			EUWorkingState.REP_COUNT = 0;
			instructionFunction = REP_STOS;
			repInstructionGetNextClockCount = getREP_STOSCycles;
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_REP_INSTRUCTION;
			return getREP_STOSCycles(EUWorkingState.REP_COUNT);

		case XED_ICLASS_CALL_NEAR:
			instructionFunction = CALL_NEAR;
			return getCALLCycles();

		case XED_ICLASS_CALL_FAR:
			instructionFunction = CALL_FAR;
			return getCALL_FARCycles();

		case XED_ICLASS_JMP:
			instructionFunction = JMP_NEAR;
			return getJMPCycles();

		case XED_ICLASS_JMP_FAR:
			instructionFunction = JMP_FAR;
			return getJMP_FARCycles();

		case XED_ICLASS_RET_NEAR:
			instructionFunction = RET_NEAR;
			return getRETCycles();

		case XED_ICLASS_RET_FAR:
			instructionFunction = RET_FAR;
			return getRET_FARCycles();

		case XED_ICLASS_JZ:
			instructionFunction = JZ;
			return getJXXCycles(cpu.getFlagStatus(CPU::ZERRO));

		case XED_ICLASS_JL:
			instructionFunction = JL;
			return getJXXCycles(cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER));

		case XED_ICLASS_JLE:
			instructionFunction = JLE;
			return getJXXCycles(cpu.getFlagStatus(CPU::ZERRO) || (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER)));

		case XED_ICLASS_JB:
			instructionFunction = JB;
			return getJXXCycles(CPU::CARRY);

		case XED_ICLASS_JBE:
			instructionFunction = JBE;
			return getJXXCycles(CPU::CARRY || CPU::ZERRO);

		case XED_ICLASS_JP:
			instructionFunction = JP;
			return getJXXCycles(CPU::PARRITY);

		case XED_ICLASS_JO:
			instructionFunction = JO;
			return getJXXCycles(cpu.getFlagStatus(CPU::OVER));
		
		case XED_ICLASS_JS:
			instructionFunction = JS;
			return getJXXCycles(cpu.getFlagStatus(CPU::SIGN));

		case XED_ICLASS_JNZ:
			instructionFunction = JNZ;
			return getJXXCycles(!cpu.getFlagStatus(CPU::ZERRO));

		case XED_ICLASS_JNL:
			instructionFunction = JNL;
			return getJXXCycles(cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER));

		case XED_ICLASS_JNLE:
			instructionFunction = JNLE;
			return getJXXCycles(!cpu.getFlagStatus(CPU::ZERRO) && (cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER)));

		case XED_ICLASS_JNB:
			instructionFunction = JNB;
			return getJXXCycles(!cpu.getFlagStatus(CPU::CARRY));

		case XED_ICLASS_JNBE:
			instructionFunction = JNBE;
			return getJXXCycles(!cpu.getFlagStatus(CPU::CARRY) && !cpu.getFlagStatus(CPU::ZERRO));

		case XED_ICLASS_JNP:
			instructionFunction = JNP;
			return getJXXCycles(!cpu.getFlagStatus(CPU::PARRITY));

		case XED_ICLASS_JNS:
			instructionFunction = JNS;
			return getJXXCycles(!cpu.getFlagStatus(CPU::SIGN));

		case XED_ICLASS_LOOP:
			instructionFunction = LOOP;
			return getLOOPCycles();
		
		case XED_ICLASS_LOOPE:// = LOOPZ
			instructionFunction = LOOPZ;
			return getLOOPZCycles();

		case XED_ICLASS_LOOPNE:// = LOOPNZ
			instructionFunction = LOOPNZ;
			return getLOOPNZCycles();

		case XED_ICLASS_JCXZ:
			instructionFunction = JCXZ;
			return getJCXZCycles();

		/* Servicing interrupts vary a bit than executing normal instruction */
		case XED_ICLASS_INT:
			cpu.interrupt(CPU::INTERRUPT_TYPE::INTERNAL, cpu.biu.instructionBufferQueue[1]);
			return 0;
		
		case XED_ICLASS_INT3:
			cpu.interrupt(CPU::INTERRUPT_TYPE::INT3);
			return 0;

		case XED_ICLASS_INTO:
			if (!cpu.getFlagStatus(CPU::OVER))
			{
				instructionFunction = doNothing;
				return 4;
			}
			cpu.interrupt(CPU::INTERRUPT_TYPE::INTO);
			return 0;

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
			instructionFunction = _CLI;
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

static unsigned int EUDecodeClock(void)
{
	xed_decoded_inst_zero_keep_mode(&cpu->decodedInst);
	const xed_error_enum_t DECODE_STATUS = xed_decode(&cpu->decodedInst,cpu.biu.instructionBufferQueue.data(), cpu.biu.instructionBufferQueuePos);

	if (DECODE_STATUS == XED_ERROR_NONE)
	{
		EUWorkingState.INSTRUCTION_CLOCK_LEFT = prepareInstructionExecution();
		// #if defined(STOP_AT_CLOCK) || defined(STOP_AT_INSTRUCTION) || defined(UNIT_TEST)
		// 	printCurrentInstruction();
		// 	#ifndef UNIT_TEST//Only regs.diregs.splay the clock cycles when not unit testing
		// 		printf("Clock cycles: %d\n",workingState.INSTRUCTION_CLOCK_LEFT);
		// 	#endif
		// #endif
		//An instruction function has been decode and the cpu will tell the EU how muregs.ch clock cycles will be needed
		return E5150::I8086::EU::STATUS_INSTRUCTION_DECODED | (E5150::I8086::EU::STATUS_INSTRUCTION_EXECUTED & (EUWorkingState.INSTRUCTION_CLOCK_LEFT == 0));
	}

	return 0;
}

unsigned int EU::clock()
{
	switch (EUWorkingState.EUWorkingMode)
	{
		case EU::WORKING_MODE::DECODE:
			return EUDecodeClock();
		
		case EU::WORKING_MODE::EXEC_INSTRUCTION:
			return EUExecInstructionClock();

		case EU::WORKING_MODE::EXEC_REP_INSTRUCTION:
			return EUExecRepInstructionClock();
		
		case EU::WORKING_MODE::EXEC_INTERRUPT_SERVICE_PROCEDURE:
			return EUExecInterruptServiceProcedureClock();
	}
}

void EU::enterInterruptServiceProcedure(const unsigned int interruptServiceClockCycles)
{
	EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_INTERRUPT_SERVICE_PROCEDURE;
	EUWorkingState.INSTRUCTION_CLOCK_LEFT = interruptServiceClockCycles;
}

void EU::farCall (const uint16_t seg, const uint16_t offset)
{
	cpu.push(cpu.regs.cs);
	cpu.push(cpu.regs.ip);

	cpu.regs.cs = seg;
	cpu.regs.ip = offset;
}

void EU::farRet (void)
{
	cpu.regs.ip = cpu.pop();
	cpu.regs.cs = cpu.pop();
}

/**
 * This function compute the effective address for a memory operand and add the correregs.sponregs.ding number of 
 * instructions cycles. The instruction cycles are picked up from https://zsmith.co/intel.php#ea :
 * 1.  regs.diregs.sp: mod = 0b00 and rm = 0b110								+6
 * 2.  (BX,BP,SI,DI): mod = 0b00 and rm != 0b110 and rm = 0b1xx		+5
 * 3.  regs.diregs.sp + (BX,BP,SI,DI): mod = 0b10 and rm = 0b1xx				+9
 * 4.1 (BP+DI, bx+SI): mod = 0b00 and rm = 0b01x					+7
 * 4.2 (BP+SI, bx+DI): mod = 0b00 and rm = 0b00x					+8
 * 5.1 regs.diregs.sp + (BP+DI, bx+SI) +-> same as precedet with mod = 0b10	+11
 * 5.2 regs.diregs.sp + (BP+SI, bx+DI) +										+12
 *
 * word operands at odd addressregs.es	+4
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
		//1. regs.diregs.sp: mod == 0b00 and rm 0b110
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
		//3. regs.diregs.sp + (base,index): mod = 0b10 rm = 0b1xx
		if (rm & 0b100)
			clockNeeded += 9;
		//5.1/5.2 base + index + regs.diregs.sp: mod = 0b10 rm = 0b01x/0b00x
		else
			clockNeeded += (rm & 0b10) ? 11 : 12;
	}

	EAddress = cpu.genAddress(xed_decoded_inst_get_seg_reg(&decodedInst,0), xed_decoded_inst_get_memory_displacement(&decodedInst,0));
	
	if (xed_operand_values_has_segment_prefix(&decodedInst))
		clockNeeded += 2;
	
	return clockNeeded;
}

const EU::InternalInfos& EU::getDebugWorkingState() { return EUWorkingState; }
