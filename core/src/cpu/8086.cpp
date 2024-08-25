#if 0
#include "core/arch.hpp"
#include "core/8086.hpp"

static bool CPU_HLT				= false;
static bool TEMP_TF				= false;
static bool INTR				= false;
static bool NMI					= false;
static bool INTO				= false;
static bool INT3				= false;
static bool INTN				= false;
static bool DIVIDE				= false;
static bool IRET_DELAY			= false;
static uint8_t INTERRUPT_VECTOR	= 0;

//TODO: 23/12/2023: Still not sure about the architecture. If I really want to separate the different components of the CPU in different class, then the EU should
// also do the initialization of the registers
CPU::CPU() : regs(), biu(), eu(), instructionExecutedCount(0)
{
	std::cout << xed_get_copyright() << std::endl;

	regs.cs = 0xFFFF;
	regs.ds = 0;
	regs.es = 0;
	regs.es = 0;
	regs.sp = 0xFF;
	regs.ip = 0;
	
	addressBus = genAddress(regs.cs, regs.ip);

	/* initializing xed */
	xed_tables_init();
}

//Clear the values of the flag, then update the values accorregs.ding to the bool status (false = 0, true = 1) using only bitwise operators to regs.speedup the operation
void CPU::updateFlag(const CPU::FLAGS_T& flag, const bool values)
{
	const unsigned int mask = (~0) * values;
	regs.flags = (regs.flags & ~flag) | (flag & mask);
}

void CPU::setFlags (const unsigned int requestedFlags)
{ regs.flags |= requestedFlags; }

void CPU::toggleFlags (const unsigned int requestedFlags)
{ regs.flags ^= requestedFlags; }

void CPU::clearFlags (const unsigned int requestedFlags)
{ regs.flags &= (~requestedFlags); }

bool CPU::getFlagStatus (const CPU::FLAGS_T flag) const
{ return regs.flags & (unsigned int) flag; }

void CPU::testCF (const unsigned int values, const bool wordSize)
{
	const unsigned int carryMask = ~(wordSize ? 0xFFFF : 0xFF);
	updateFlag(CARRY,values & carryMask);
}

void CPU::testPF (unsigned int values)
{
	unsigned int count = 1;

	while (values & 0xFF)
	{
		count += values & 1;
		values >>= 1;
	}
	updateFlag(PARRITY, count & 1);
}

//TODO: need to be tested
void CPU::testAF (const unsigned int values)
{ updateFlag(A_CARRY, values > (~0b111)); }

void CPU::testZF (const unsigned int values)
{ updateFlag(ZERRO,values == 0); }

void CPU::testSF (const unsigned int values)
{ updateFlag(SIGN, values & (1 << (sizeof(unsigned int) - 1))); }

void CPU::testOF (const unsigned int values, const bool wordSize)
{
	const bool newFlagValues = wordSize ? (values & (~0xFFFF)) : (values & (~0xFF));
	updateFlag(OVER,newFlagValues);
}

void CPU::updateStatusFlags (const unsigned int values, const bool wordSize)
{
	testCF(values, wordSize);
	testPF(values);
	testAF(values);
	testZF(values);
	testSF(values);
	testOF(values, wordSize);
}

unsigned int CPU::genAddress (const uint16_t base, const xed_reg_enum_t offset) const
{ return genAddress(base, readReg(offset)); }

unsigned int CPU::genAddress (const xed_reg_enum_t segment, const uint16_t offset) const
{ return genAddress(readReg(segment), offset); }

unsigned int CPU::genAddress (const xed_reg_enum_t segment, const xed_reg_enum_t offset) const
{return genAddress(readReg(segment), readReg(offset));}

uint16_t CPU::readReg(const xed_reg_enum_t reg) const
{
	switch (reg)
	{
	case XED_REG_AX:
		return regs.ax;

	case XED_REG_BX:
		return regs.bx;

	case XED_REG_CX:
		return regs.cx;

	case XED_REG_DX:
		return regs.dx;

	case XED_REG_AH:
		return regs.ah;

	case XED_REG_BH:
		return regs.bh;

	case XED_REG_CH:
		return regs.ch;

	case XED_REG_DH:
		return regs.dh;

	case XED_REG_AL:
		return regs.al;

	case XED_REG_BL:
		return regs.bl;

	case XED_REG_CL:
		return regs.cl;

	case XED_REG_DL:
		return regs.dl;

	case XED_REG_SI:
		return regs.si;

	case XED_REG_DI:
		return regs.di;

	case XED_REG_BP:
		return regs.bp;

	case XED_REG_SP:
		return regs.sp;

	case XED_REG_CS:
		return regs.cs;

	case XED_REG_DS:
		return regs.ds;

	case XED_REG_ES:
		return regs.es;

	case XED_REG_SS:
		return regs.es;
	}

	// Should never be rearegs.ched but here to regs.silent compiler warning
	// TODO: launregs.ch an exception here ? (probaregs.bly yregs.es, it is not ok if the program goregs.es here)
	return 0;
}

void CPU::interrupt(const CPU::INTERRUPT_TYPE type, const uint8_t interruptVector)
{
	INTERRUPT_VECTOR = interruptVector;

	switch (type)
	{
		case CPU::INTERRUPT_TYPE::EXTERNAL:
			INTR = true;
			return;
		
		case CPU::INTERRUPT_TYPE::INTERNAL:
			INTN = true;
			return;
		
		case CPU::INTERRUPT_TYPE::NMI:
			NMI = true;
			return;
		
		case CPU::INTERRUPT_TYPE::INTO:
			INTO = true;
			return;
		
		case CPU::INTERRUPT_TYPE::INT3:
			INT3 = true;
			return;
		
		case CPU::INTERRUPT_TYPE::DIVIDE:
			DIVIDE = true;
			return;

		#if DEBUG_BUILD
		default:
			ERROR("Interrupt type not hanregs.dled. Program quit\n");
			*(volatile unsigned int*)0 = 4;//Force error emisregs.sion (by writing to memory address 0) if an interrupt type is not hanregs.dled
		#endif
	}
}

void CPU::handleInterrupts(void)
{
	static unsigned int INTERRUPT_SEQUENCE_CLOCK_COUNT = 0;
	CPU_HLT = false;
	if (INTN)
	{
		INTN = false;
		INTERRUPT_SEQUENCE_CLOCK_COUNT = 51;
	}
	else if (INT3)// Generregs.al int n interrupt
	{
		INT3 = false;
		INTERRUPT_SEQUENCE_CLOCK_COUNT = 52;
		INTERRUPT_VECTOR = 3;
	}
	else if (DIVIDE)
	{
		DIVIDE = false;
		INTERRUPT_SEQUENCE_CLOCK_COUNT = 51;//Guregs.eregs.esed TODO: searregs.ch the clock count for now I aregs.esume it is equivregs.alent to int 0
		INTERRUPT_VECTOR = 0;
	}
	else if (INTO)
	{
		INTO = false;
		INTERRUPT_SEQUENCE_CLOCK_COUNT = 53;
		INTERRUPT_VECTOR = 4;
	}
	else if (NMI)// nmi
	{
		NMI = false;
		INTERRUPT_VECTOR = 2;
	}
	else if (INTR)// intr line aregs.eserted
	{
		INTR = false;
		if (!cpu.getFlagStatus(CPU::FLAGS_T::INTF))
		{
			EMULATION_INFO_LOG<EMULATION_MAX_LOG_LEVEL>("INTEL 8088: INTERRUPT: interrupt requregs.est while IF is regs.disaregs.bled");
			return;
		}
		INTERRUPT_SEQUENCE_CLOCK_COUNT = 61;
	}
	else//trap interrupt
	{
		INTERRUPT_VECTOR = 1;
		INTERRUPT_SEQUENCE_CLOCK_COUNT = 50;
	}
	
	cpu.biu.startInterruptDataSaveSequence();
	cpu.eu.enterInterruptServiceProcedure(INTERRUPT_SEQUENCE_CLOCK_COUNT);
}

bool CPU::interruptSequence()
{
	push(regs.flags);
	cpu.eu.farCall(biu.readWord(INTERRUPT_VECTOR*4 + 2), biu.readWord(INTERRUPT_VECTOR*4));
	TEMP_TF = getFlagStatus(CPU::FLAGS_T::TRAP);
	clearFlags(CPU::FLAGS_T::INTF | CPU::FLAGS_T::TRAP);

	return NMI || TEMP_TF;
}

void CPU::iretDelay(void) { IRET_DELAY = true; }

unsigned int CPU::clock()
{
	if (!CPU_HLT)
		cpu.biu.clock();
	const unsigned int EUStatus = cpu.eu.clock();
	if (EUStatus & E5150::I8086::EU::STATUS_INSTRUCTION_EXECUTED)
	{
		biu.instructionBufferQueuePop(cpu.eu.instructionLength);
		instructionExecutedCount += 1;

		//TODO: This might be optimized by using bit mask to tell which interrupt is triggered
		const bool shouldInterrupt = (!IRET_DELAY) && (INTR || INTN || INTO || INT3 || DIVIDE || NMI || cpu.getFlagStatus(CPU::FLAGS_T::TRAP));
		if (shouldInterrupt)
			handleInterrupts();
		IRET_DELAY = false;
	}

	biu.updateClockFunction();
	return EUStatus;
}

void CPU::hlt(void) { CPU_HLT = true; }

void CPU::push (const uint16_t data)
{
	regs.sp -= 2;
	cpu.biu.writeWord(cpu.genAddress(cpu.regs.es,regs.sp), data);
}

uint16_t CPU::pop (void)
{
	const uint16_t ret = cpu.biu.readWord(cpu.genAddress(cpu.regs.es,regs.sp));
	regs.sp += 2;
	return ret;
}

void CPU::write_reg(const xed_reg_enum_t reg, const unsigned int data)
{
	switch (reg)
	{
	case XED_REG_AX:
		regs.ax = data;
		break;

	case XED_REG_BX:
		regs.bx = data;
		break;

	case XED_REG_CX:
		regs.cx = data;
		break;

	case XED_REG_DX:
		regs.dx = data;
		break;

	case XED_REG_AH:
		regs.ah = data;
		break;

	case XED_REG_BH:
		regs.bh = data;
		break;

	case XED_REG_CH:
		regs.ch = data;
		break;

	case XED_REG_DH:
		regs.dh = data;
		break;

	case XED_REG_AL:
		regs.al = data;
		break;

	case XED_REG_BL:
		regs.bl = data;
		break;

	case XED_REG_CL:
		regs.cl = data;
		break;

	case XED_REG_DL:
		regs.dl = data;
		break;

	case XED_REG_SI:
		regs.si = data;
		break;

	case XED_REG_DI:
		regs.di = data;
		break;

	case XED_REG_BP:
		regs.bp = data;
		break;

	case XED_REG_SP:
		regs.sp = data;
		break;

	case XED_REG_CS:
		regs.cs = data;
		break;

	case XED_REG_DS:
		regs.ds = data;
		break;

	case XED_REG_ES:
		regs.es = data;
		break;

	case XED_REG_SS:
		regs.es = data;
		break;
	}
}
#endif

#include "core/arch.hpp"
#include "core/instructions.hpp"

E5150::Intel8088::Intel8088():
		mode(Intel8088::ERunningMode::INIT_SEQUENCE), instructionStreamQueueIndex(0),
		biuClockCountDown(MEMORY_FETCH_CLOCK_COUNT), euClockCount(0)
{
	xed_tables_init();
}

static constexpr unsigned int INIT_SEQUENCE_CLOCK_COUNT = 11;
static void CPUClock_Init(E5150::Intel8088* cpu)
{
	cpu->clock += 1;

	if (cpu->clock == INIT_SEQUENCE_CLOCK_COUNT)
	{
		cpu->regs.cs = 0xFFFF;
		cpu->regs.es = 0;
		cpu->regs.ds = 0;
		cpu->regs.es = 0;
		cpu->regs.sp = 0xFF;
		cpu->regs.ip = 0;
		cpu->mode = E5150::Intel8088::ERunningMode::OPERATIONAL;
	}
}

static void BIUMode_Switch(E5150::Intel8088* cpu, const E5150::Intel8088::EBIURunningMode newMode)
{
	switch (newMode)
	{
		case E5150::Intel8088::EBIURunningMode::FETCH_MEMORY:
			cpu->biuClockCountDown = E5150::Intel8088::MEMORY_FETCH_CLOCK_COUNT;
			break;
		default:
			break;
	}

	cpu->biuMode = newMode;
}

static unsigned int GenerateFetAddress (const uint16_t cs, const uint16_t ip)
{
	return (cs << 4) + ip;
}

static void BIUClock_FetchInstruction(E5150::Intel8088* cpu)
{
	cpu->biuClockCountDown -= 1;

	if (cpu->biuClockCountDown == 0)
	{
		const uint8_t byte = E5150::Arch::ram.Read(GenerateFetAddress(cpu->regs.cs,cpu->regs.ip));
		cpu->regs.ip += 1;
		cpu->instructionStreamQueue[cpu->instructionStreamQueueIndex++] = byte;

		if (cpu->instructionStreamQueueIndex == E5150::Intel8088::INSTRUCTION_STREAM_QUEUE_LENGTH)
		{
			BIUMode_Switch(cpu,E5150::Intel8088::EBIURunningMode::WAIT_ROOM_IN_QUEUE);
		}
		else
		{
			cpu->biuClockCountDown = E5150::Intel8088::MEMORY_FETCH_CLOCK_COUNT;
		}
	}
}

static void BIUClock_FetchMemory(E5150::Intel8088* cpu)
{

}

static void BIUClock_WaitRoomInQueue(E5150::Intel8088* cpu)
{
	if (cpu->instructionStreamQueueIndex < E5150::Intel8088::INSTRUCTION_STREAM_QUEUE_INDEX_MAX)
	{
		BIUMode_Switch(cpu,E5150::Intel8088::EBIURunningMode::FETCH_MEMORY);

		// Since the BIU clock simulation function is called before the EU clock simulation function and since the EU
		// clock simulation function will be the one responsible for decreasing the instruction stream index
		// the detection of new room available is done in the next clock simulation step, so we need to simulate a fetch
		// cycle for this simulation step
		BIUClock_FetchInstruction(cpu);
	}
}

static void BIUClock_Simulate(E5150::Intel8088* cpu)
{
	switch (cpu->biuMode)
	{
		case E5150::Intel8088::EBIURunningMode::FETCH_INSTRUCTION:
			BIUClock_FetchInstruction(cpu);
			break;
		case E5150::Intel8088::EBIURunningMode::FETCH_MEMORY:
			BIUClock_FetchMemory(cpu);
			break;
		case E5150::Intel8088::EBIURunningMode::WAIT_ROOM_IN_QUEUE:
			BIUClock_WaitRoomInQueue(cpu);
			break;
		default:
			assert(true);//All running mode must be simulated
	}
}

static unsigned int PrepareInstructionExecution (E5150::Intel8088* cpu, bool* requestMemory)
{
	//At the end of the opcode of instructions that access memory, there is w bit = 0 for byte operand and 1 one for word operands.
	//If this bit = 0 there is 1 memory access and if it = 1, 2 memory accesses
	const unsigned int nPrefix = xed_decoded_inst_get_nprefixes(&cpu->decodedInst);
	const unsigned int operandSizeWord = cpu->instructionStreamQueue[nPrefix] & 0b1;
	const unsigned int memoryByteAccess = xed_decoded_inst_number_of_memory_operands(&cpu->decodedInst) * (operandSizeWord + 1);
	*requestMemory = memoryByteAccess != 0;
#if 0
	cpu.biu.requestMemoryByte(memoryByteAccess);
	cpu.eu.instructionLength = xed_decoded_inst_get_length(&cpu->decodedInst);
	EUWorkingState.CURRENT_INSTRUCTION_CS = cpu.regs.cs;
	EUWorkingState.CURRENT_INSTRUCTION_IP = cpu.regs.ip;
	cpu.biu.IPToNextInstruction(cpu.eu.instructionLength);
	EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_INSTRUCTION;
#endif

	const xed_iclass_enum_t decodedInstGetIclass = xed_decoded_inst_get_iclass(&cpu->decodedInst);
	cpu->decodedInstructionIClass = decodedInstGetIclass;

	switch (decodedInstGetIclass)
	{
		case XED_ICLASS_MOV:
			return 11;//getMOVCycles();

#if 0
		case XED_ICLASS_PUSH:
			return getPUSHCycles();

		case XED_ICLASS_POP:
			return getPOPCycles();

		case XED_ICLASS_XCHG:
			return getXCHGCycles();

		case XED_ICLASS_IN:
			return getINCycles();

		case XED_ICLASS_OUT:
			return getOUTCycles();

		case XED_ICLASS_XLAT:
			return getXLATCycles();

		case XED_ICLASS_LEA:
			return getLEACycles();

		case XED_ICLASS_LDS:
			return getLDSCycles();

		case XED_ICLASS_LES:
			return getLESCycles();

		case XED_ICLASS_LAHF:
			return getLAHFCycles();

		case XED_ICLASS_SAHF:
			return getSAHFCycles();

		case XED_ICLASS_PUSHF:
			return getPUSHFCycles();

		case XED_ICLASS_POPF:
			return getPOPFCycles();

		case XED_ICLASS_ADD:
			return getADDCycles();

		case XED_ICLASS_ADC:
			return getADDCycles();

		case XED_ICLASS_INC:
			return getINCCycles();

		case XED_ICLASS_AAA:
			return getAAACycles();

		case XED_ICLASS_DAA:
			return getDAACycles();

		case XED_ICLASS_SUB:
			return getSUBCycles();

		case XED_ICLASS_SBB:
			return getSUBCycles();

		case XED_ICLASS_DEC:
			return getDECCycles();

		case XED_ICLASS_NEG:
			return getNEGCycles();

		case XED_ICLASS_CMP:
			return getCMPCycles();

		case XED_ICLASS_AAS:
			return getAASCycles();

		case XED_ICLASS_DAS:
			return getDASCycles();

		case XED_ICLASS_MUL:
			return getMULCycles();

		case XED_ICLASS_IMUL:
			return getIMULCycles();

		case XED_ICLASS_DIV:
			return getDIVCycles();

		case XED_ICLASS_IDIV:
			return getIDIVCycles();

		case XED_ICLASS_AAD:
			return getAADCycles();

		case XED_ICLASS_CBW:
			return getCBWCycles();

		case XED_ICLASS_CWD:
			return getCWDCycles();

		case XED_ICLASS_NOT:
			return getNOTCycles();

		case XED_ICLASS_SHL:
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_SHR:
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_SAR:
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_ROL:
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_ROR:
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_RCL:
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_RCR:
			return getSHIFT_ROTATECycles(nPrefix);

		case XED_ICLASS_AND:
			return getANDCycles();

		case XED_ICLASS_TEST:
			return getTESTCycles();

		case XED_ICLASS_OR:
			return getORCycles();

		case XED_ICLASS_XOR:
			return getXORCycles();

		case XED_ICLASS_MOVSB:
		case XED_ICLASS_MOVSW:
			return getMOVSCycles();

		case XED_ICLASS_REP_MOVSB:
		case XED_ICLASS_REP_MOVSW:
#if 0
			EUWorkingState.REP_COUNT = 0;
			repInstructionGetNextClockCount = getREP_MOVSCycles;
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_REP_INSTRUCTION;
			return getREP_MOVSCycles(EUWorkingState.REP_COUNT);
#else
			return 0;
#endif

		case XED_ICLASS_CMPSB:
		case XED_ICLASS_CMPSW:
			return getCMPSCycles();

		case XED_ICLASS_REPE_CMPSB:
		case XED_ICLASS_REPNE_CMPSB:
		case XED_ICLASS_REPE_CMPSW:
		case XED_ICLASS_REPNE_CMPSW:
#if 0
			EUWorkingState.REP_COUNT = 0;
			repInstructionGetNextClockCount = getREP_CMPSCycles;
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_REP_INSTRUCTION;
			return getREP_CMPSCycles(EUWorkingState.REP_COUNT);
#else
			return 10;
#endif

		case XED_ICLASS_SCASB:
		case XED_ICLASS_SCASW:
			return getSCASCycles();

		case XED_ICLASS_REPE_SCASB:
		case XED_ICLASS_REPNE_SCASB:
		case XED_ICLASS_REPE_SCASW:
		case XED_ICLASS_REPNE_SCASW:
#if 0
			EUWorkingState.REP_COUNT = 0;
			repInstructionGetNextClockCount = getREP_SCASCycles;
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_REP_INSTRUCTION;
			return getREP_SCASCycles(EUWorkingState.REP_COUNT);
#else
			return 10;
#endif

		case XED_ICLASS_LODSB:
		case XED_ICLASS_LODSW:
			return getLODSCycles();

		case XED_ICLASS_REP_LODSB:
		case XED_ICLASS_REP_LODSW:
#if 0
			EUWorkingState.REP_COUNT = 0;
			repInstructionGetNextClockCount = getREP_LODSCycles;
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_REP_INSTRUCTION;
			return getREP_LODSCycles(EUWorkingState.REP_COUNT);
#else
			return 10;
#endif
		case XED_ICLASS_STOSB:
		case XED_ICLASS_STOSW:
			return getSTOSCycles();

		case XED_ICLASS_REP_STOSB:
		case XED_ICLASS_REP_STOSW:
#if 0
			EUWorkingState.REP_COUNT = 0;
			repInstructionGetNextClockCount = getREP_STOSCycles;
			EUWorkingState.EUWorkingMode = EU::WORKING_MODE::EXEC_REP_INSTRUCTION;
			return getREP_STOSCycles(EUWorkingState.REP_COUNT);
#else
			return 10;
#endif
		case XED_ICLASS_CALL_NEAR:
			return getCALLCycles();

		case XED_ICLASS_CALL_FAR:
			return getCALL_FARCycles();

		case XED_ICLASS_JMP:
			return getJMPCycles();

		case XED_ICLASS_JMP_FAR:
			return getJMP_FARCycles();

		case XED_ICLASS_RET_NEAR:
			return getRETCycles();

		case XED_ICLASS_RET_FAR:
			return getRET_FARCycles();

		case XED_ICLASS_JZ:
#if 0
			return getJXXCycles(cpu.getFlagStatus(CPU::ZERRO));
#else
			return 10;
#endif
		case XED_ICLASS_JL:
#if 0
			return getJXXCycles(cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER));
#else
			return 10;
#endif
		case XED_ICLASS_JLE:
#if 0
			return getJXXCycles(cpu.getFlagStatus(CPU::ZERRO) || (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER)));
#else
			return 10;
#endif
		case XED_ICLASS_JB:
#if 0
			return getJXXCycles(CPU::CARRY);
#else
			return 10;
#endif
		case XED_ICLASS_JBE:
#if 0
			return getJXXCycles(CPU::CARRY || CPU::ZERRO);
#else
			return 10;
#endif
		case XED_ICLASS_JP:
#if 0
			return getJXXCycles(CPU::PARRITY);
#else
			return 10;
#endif
		case XED_ICLASS_JO:
#if 0
			return getJXXCycles(cpu.getFlagStatus(CPU::OVER));
#else
			return 10;
#endif
		case XED_ICLASS_JS:
#if 0
			return getJXXCycles(cpu.getFlagStatus(CPU::SIGN));
#else
			return 10;
#endif
		case XED_ICLASS_JNZ:
#if 0
			return getJXXCycles(!cpu.getFlagStatus(CPU::ZERRO));
#else
			return 10;
#endif
		case XED_ICLASS_JNL:
#if 0
			return getJXXCycles(cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER));
#else
			return 10;
#endif
		case XED_ICLASS_JNLE:
#if 0
			return getJXXCycles(!cpu.getFlagStatus(CPU::ZERRO) && (cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER)));
#else
			return 10;
#endif
		case XED_ICLASS_JNB:
#if 0
			return getJXXCycles(!cpu.getFlagStatus(CPU::CARRY));
#else
			return 10;
#endif
		case XED_ICLASS_JNBE:
#if 0
			return getJXXCycles(!cpu.getFlagStatus(CPU::CARRY) && !cpu.getFlagStatus(CPU::ZERRO));
#else
			return 10;
#endif
		case XED_ICLASS_JNP:
#if 0
			return getJXXCycles(!cpu.getFlagStatus(CPU::PARRITY));
#else
			return 10;
#endif
		case XED_ICLASS_JNS:
#if 0
			return getJXXCycles(!cpu.getFlagStatus(CPU::SIGN));
#else
			return 10;
#endif
		case XED_ICLASS_LOOP:
			return getLOOPCycles();

		case XED_ICLASS_LOOPE:// = LOOPZ
			return getLOOPZCycles();

		case XED_ICLASS_LOOPNE:// = LOOPNZ
			return getLOOPNZCycles();

		case XED_ICLASS_JCXZ:
			return getJCXZCycles();

			/* Servicing interrupts vary a bit than executing normal instruction */
		case XED_ICLASS_INT:
#if 0
			cpu.interrupt(CPU::INTERRUPT_TYPE::INTERNAL, cpu.biu.instructionBufferQueue[1]);
#endif
			return 0;

		case XED_ICLASS_INT3:
#if 0
			cpu.interrupt(CPU::INTERRUPT_TYPE::INT3);
#endif
			return 0;

		case XED_ICLASS_INTO:
#if 0
			if (!cpu.getFlagStatus(CPU::OVER))
			{
				return 4;
			}
			cpu.interrupt(CPU::INTERRUPT_TYPE::INTO);
#endif
			return 0;

		case XED_ICLASS_IRET:
			return getIRETCycles();

		case XED_ICLASS_CLC:
			return getCLCCycles();

		case XED_ICLASS_CMC:
			return getCMCCycles();

		case XED_ICLASS_STC:
			return getSTCCycles();

		case XED_ICLASS_CLD:
			return getCLDCycles();

		case XED_ICLASS_STD:
			return getSTDCycles();

		case XED_ICLASS_CLI:
			return getCLICycles();

		case XED_ICLASS_STI:
			return getSTICycles();

		case XED_ICLASS_HLT:
			return getHLTCycles();

		case XED_ICLASS_NOP:
			return getNOPCycles();
#endif

		default:
			spdlog::debug("Instruction not recognized yet");
			return 10;
	}
}

static void EUClock_WaitInstruction(E5150::Intel8088* cpu)
{
	xed_decoded_inst_zero_keep_mode(&cpu->decodedInst);
	xed_error_enum_t status = xed_decode(&cpu->decodedInst,
										 cpu->instructionStreamQueue,
										 cpu->instructionStreamQueueIndex);
	if (status == XED_ERROR_NONE)
	{
		bool requestMemory;
		cpu->instructionStreamQueueIndex -= xed_decoded_inst_get_length(&cpu->decodedInst);
		cpu->euClockCount = PrepareInstructionExecution(cpu,&requestMemory);
		cpu->events |= (int)E5150::Intel8088::EEventFlags::INSTRUCTION_DECODED;
		cpu->euMode = requestMemory ?
						E5150::Intel8088::EEURunningMode::WAIT_BIU :
						E5150::Intel8088::EEURunningMode::EXECUTE_INSTRUCTION;
	}
}

static void ExecuteInstruction(E5150::Intel8088* cpu)
{
	switch (cpu->decodedInstructionIClass)
	{
		case XED_ICLASS_MOV:
			//MOV(cpu);
			break;
#if 0
		case XED_ICLASS_PUSH:
			PUSH(cpu);
			break;

		case XED_ICLASS_POP:
			POP(cpu);
			break;

		case XED_ICLASS_XCHG:
			XCHG(cpu);
			break;

		case XED_ICLASS_IN:
			_IN(cpu);
			break;

		case XED_ICLASS_OUT:
			_OUT(cpu);
			break;

		case XED_ICLASS_XLAT:
			XLAT(cpu);
			break;

		case XED_ICLASS_LEA:
			LEA(cpu);
			break;

		case XED_ICLASS_LDS:
			LDS(cpu);
			break;

		case XED_ICLASS_LES:
			LES(cpu);
			break;

		case XED_ICLASS_LAHF:
			LAHF(cpu);
			break;

		case XED_ICLASS_SAHF:
			SAHF(cpu);
			break;

		case XED_ICLASS_PUSHF:
			PUSHF(cpu);
			break;

		case XED_ICLASS_POPF:
			POPF(cpu);
			break;

		case XED_ICLASS_ADD:
#if 0
			cpu.eu.instructionExtraData.clearData();
#endif
			ADD(cpu);
			break;

		case XED_ICLASS_ADC:
#if 0
			cpu.eu.instructionExtraData.clearData();
#endif
			ADD(cpu);
			break;

		case XED_ICLASS_INC:
			INC(cpu);
			break;

		case XED_ICLASS_AAA:
			AAA(cpu);
			break;

		case XED_ICLASS_DAA:
			DAA(cpu);
			break;

		case XED_ICLASS_SUB:
#if 0
			cpu.eu.instructionExtraData.clearData();
#endif
			SUB(cpu);
			break;

		case XED_ICLASS_SBB:
#if 0
			cpu.eu.instructionExtraData.clearData();
#endif
			SUB(cpu);
			break;

		case XED_ICLASS_DEC:
			DEC(cpu);
			break;

		case XED_ICLASS_NEG:
			NEG(cpu);
			break;

		case XED_ICLASS_CMP:
			CMP(cpu);
			break;

		case XED_ICLASS_AAS:
			AAS(cpu);
			break;

		case XED_ICLASS_DAS:
			DAS(cpu);
			break;

		case XED_ICLASS_MUL:
#if 0
			cpu.eu.instructionExtraData.clearData();
#endif
			MUL(cpu);
			break;

		case XED_ICLASS_IMUL:
#if 0
			cpu.eu.instructionExtraData.isSigned = true;
#endif
			MUL(cpu);
			break;

		case XED_ICLASS_DIV:
#if 0
			cpu.eu.instructionExtraData.clearData();
#endif
			DIV(cpu);
			break;

		case XED_ICLASS_IDIV:
#if 0
			cpu.eu.instructionExtraData.isSigned = true;
#endif
			DIV(cpu);
			break;

		case XED_ICLASS_AAD:
			AAD(cpu);
			break;

		case XED_ICLASS_CBW:
			CBW(cpu);
			break;

		case XED_ICLASS_CWD:
			CWD(cpu);
			break;

		case XED_ICLASS_NOT:
			NOT(cpu);
			break;

		case XED_ICLASS_SHL:
#if 0
			cpu.eu.instructionExtraData.clearData();
			cpu.eu.instructionExtraData.setDirectionIsLeft();
#endif
			SHIFT(cpu);
			break;

		case XED_ICLASS_SHR:
#if 0
			cpu.eu.instructionExtraData.clearData();
#endif
			SHIFT(cpu);
			break;

		case XED_ICLASS_SAR:
#if 0
			cpu.eu.instructionExtraData.clearData();
			cpu.eu.instructionExtraData.setInstructionIsArithmetic();
#endif
			SHIFT(cpu);
			break;

		case XED_ICLASS_ROL:
#if 0
			cpu.eu.instructionExtraData.clearData();
			cpu.eu.instructionExtraData.setDirectionIsLeft();
#endif
			ROTATE(cpu);
			break;

		case XED_ICLASS_ROR:
			ROTATE(cpu);
			break;

		case XED_ICLASS_RCL:
#if 0
			cpu.eu.instructionExtraData.clearData();
			cpu.eu.instructionExtraData.setRotationWithCarry();
			cpu.eu.instructionExtraData.setDirectionIsLeft();
#endif
			ROTATE(cpu);
			break;

		case XED_ICLASS_RCR:
#if 0
			cpu.eu.instructionExtraData.clearData();
			cpu.eu.instructionExtraData.setRotationWithCarry();
#endif
			ROTATE(cpu);
			break;

		case XED_ICLASS_AND:
			AND(cpu);
			break;

		case XED_ICLASS_TEST:
			TEST(cpu);
			break;

		case XED_ICLASS_OR:
			OR(cpu);
			break;

		case XED_ICLASS_XOR:
			XOR(cpu);
			break;

		case XED_ICLASS_MOVSB:
		case XED_ICLASS_MOVSW:
			MOVS(cpu);
			break;

//TODO: A copy past error happened for the REP isntructions, investigate from the eu.cpp file what has to be done
		case XED_ICLASS_REP_MOVSB:
		case XED_ICLASS_REP_MOVSW:
			REP_MOVS(cpu);
			break;

		case XED_ICLASS_CMPSB:
		case XED_ICLASS_CMPSW:
			CMPS(cpu);
			break;

		case XED_ICLASS_REPE_CMPSB:
		case XED_ICLASS_REPNE_CMPSB:
		case XED_ICLASS_REPE_CMPSW:
		case XED_ICLASS_REPNE_CMPSW:
			REP_CMPS(cpu);
			break;

		case XED_ICLASS_SCASB:
		case XED_ICLASS_SCASW:
			SCAS(cpu);
			break;

		case XED_ICLASS_REPE_SCASB:
		case XED_ICLASS_REPNE_SCASB:
		case XED_ICLASS_REPE_SCASW:
		case XED_ICLASS_REPNE_SCASW:
			REP_SCAS(cpu);
			break;

		case XED_ICLASS_LODSB:
		case XED_ICLASS_LODSW:
			LODS(cpu);
			break;

		case XED_ICLASS_REP_LODSB:
		case XED_ICLASS_REP_LODSW:
			REP_LODS(cpu);
			break;

		case XED_ICLASS_STOSB:
		case XED_ICLASS_STOSW:
			STOS(cpu);
			break;

		case XED_ICLASS_REP_STOSB:
		case XED_ICLASS_REP_STOSW:
			REP_STOS(cpu);
			break;

		case XED_ICLASS_CALL_NEAR:
			CALL_NEAR(cpu);
			break;

		case XED_ICLASS_CALL_FAR:
			CALL_FAR(cpu);
			break;

		case XED_ICLASS_JMP:
			JMP_NEAR(cpu);
			break;

		case XED_ICLASS_JMP_FAR:
			JMP_FAR(cpu);
			break;

		case XED_ICLASS_RET_NEAR:
			RET_NEAR(cpu);
			break;

		case XED_ICLASS_RET_FAR:
			RET_FAR(cpu);
			break;

		case XED_ICLASS_JZ:
			JZ(cpu);
			break;

		case XED_ICLASS_JL:
			JL(cpu);
			break;

		case XED_ICLASS_JLE:
			JLE(cpu);
			break;

		case XED_ICLASS_JB:
			JB(cpu);
			break;

		case XED_ICLASS_JBE:
			JBE(cpu);
			break;

		case XED_ICLASS_JP:
			JP(cpu);
			break;

		case XED_ICLASS_JO:
			JO(cpu);
			break;

		case XED_ICLASS_JS:
			JS(cpu);
			break;

		case XED_ICLASS_JNZ:
			JNZ(cpu);
			break;

		case XED_ICLASS_JNL:
			JNL(cpu);
			break;

		case XED_ICLASS_JNLE:
			JNLE(cpu);
			break;

		case XED_ICLASS_JNB:
			JNB(cpu);
			break;

		case XED_ICLASS_JNBE:
			JNBE(cpu);
			break;

		case XED_ICLASS_JNP:
			JNP(cpu);
			break;

		case XED_ICLASS_JNS:
			JNS(cpu);
			break;

		case XED_ICLASS_LOOP:
			LOOP(cpu);
			break;

		case XED_ICLASS_LOOPE:// = LOOPZ
			LOOPZ(cpu);
			break;

		case XED_ICLASS_LOOPNE:// = LOOPNZ
			LOOPNZ(cpu);
			break;

		case XED_ICLASS_JCXZ:
			JCXZ(cpu);
			break;

			/* Servicing interrupts vary a bit than executing normal instruction */
		case XED_ICLASS_INT:
#if 0
			cpu.interrupt(CPU::INTERRUPT_TYPE::INTERNAL, cpu.biu.instructionBufferQueue[1]);
			return 0;
#else
			break;
#endif

		case XED_ICLASS_INT3:
#if 0
			cpu.interrupt(CPU::INTERRUPT_TYPE::INT3);
			return 0;
#else
			break;
#endif

		case XED_ICLASS_INTO:
#if 0
			if (!cpu.getFlagStatus(CPU::OVER))
			{
				doNothing(cpu);
				break;
			}
			cpu.interrupt(CPU::INTERRUPT_TYPE::INTO);
			return 0;
#else
			break;
#endif

		case XED_ICLASS_IRET:
			IRET(cpu);
			break;

		case XED_ICLASS_CLC:
			CLC(cpu);
			break;

		case XED_ICLASS_CMC:
			CMC(cpu);
			break;

		case XED_ICLASS_STC:
			STC(cpu);
			break;

		case XED_ICLASS_CLD:
			CLD(cpu);
			break;

		case XED_ICLASS_STD:
			STD(cpu);
			break;

		case XED_ICLASS_CLI:
			_CLI(cpu);
			break;

		case XED_ICLASS_STI:
			STI(cpu);
			break;

		case XED_ICLASS_HLT:
			HLT(cpu);
			break;

		case XED_ICLASS_NOP:
			NOP(cpu);
			break;
#endif

		default:
			spdlog::debug("Instruction not simulated yet");
			//assert(false);
	}

}

static void EUClock_ExecuteInstruction(E5150::Intel8088* cpu)
{
	cpu->euClockCount -= 1;

	if (cpu->euClockCount == 0)
	{
		ExecuteInstruction(cpu);
		cpu->events |= (int)E5150::Intel8088::EEventFlags::INSTRUCTION_EXECUTED;
		cpu->euMode = E5150::Intel8088::EEURunningMode::WAIT_INSTRUCTION;
	}
}

static void EUClock_WaitBIU(E5150::Intel8088* cpu)
{
	if (cpu->biuMode == E5150::Intel8088::EBIURunningMode::FETCH_MEMORY)
	{
		EUClock_ExecuteInstruction(cpu);
		cpu->euMode = E5150::Intel8088::EEURunningMode::EXECUTE_INSTRUCTION;
	}
}

static void EUClock_Simulate(E5150::Intel8088* cpu)
{
	switch (cpu->euMode)
	{
		case E5150::Intel8088::EEURunningMode::WAIT_INSTRUCTION:
			EUClock_WaitInstruction(cpu);
			break;

		case E5150::Intel8088::EEURunningMode::EXECUTE_INSTRUCTION:
			EUClock_ExecuteInstruction(cpu);
			break;

		case E5150::Intel8088::EEURunningMode::WAIT_BIU:
			EUClock_WaitBIU(cpu);
			break;

		default:
			assert(false);
	}
}

static void CPUClock_Operational(E5150::Intel8088* cpu)
{
	BIUClock_Simulate(cpu);
	EUClock_Simulate(cpu);
}

void E5150::Intel8088::Clock()
{
	if (halted)
	{
		return;
	}
	this->events = 0;
	switch (mode) {
		case ERunningMode::INIT_SEQUENCE:
			CPUClock_Init(this);
			break;

		case ERunningMode::OPERATIONAL:
			CPUClock_Operational(this);
			break;
	}
}
