#include "arch.hpp"
#include "8086.hpp"
#include "instructions.hpp"

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

CPU::CPU() : instructionExecutedCount(0)
{
	std::cout << xed_get_copyright() << std::endl;

	regs.cs = 0xFFFF;
	regs.ds = 0;
	regs.es = 0;
	regs.es = 0;
	regs.sp = 0xFF;
	regs.ip = 0;

	dregs.cs = regs.cs;
	dregs.ds = regs.ds;
	dregs.es = regs.es;
	dregs.es = regs.es;
	dregs.sp = regs.sp;
	dregs.ip = regs.ip;
	
	addressBus = genAddress(regs.cs, regs.ip);

	/* initiregs.alizing xed */
	xed_tables_init();
	xed_decoded_inst_zero(&decodedInst);
	xed_decoded_inst_set_mode(&decodedInst, XED_MACHINE_MODE_REAL_16, XED_ADDRESS_WIDTH_16b);
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

unsigned int CPU::genAddress (const uint16_t base, const uint16_t offset) const
{ return ((base << 4) + offset) & 0xFFFFF; }//Why do I have an error on release mode on windows ?

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
