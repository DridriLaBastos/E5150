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

	cs = 0xFFFF;
	ds = 0;
	ss = 0;
	es = 0;
	sp = 0xFF;
	ip = 0;
	addressBus = genAddress(cs,ip);

	/* initializing xed */
	xed_tables_init();
	xed_decoded_inst_zero(&decodedInst);
	xed_decoded_inst_set_mode(&decodedInst, XED_MACHINE_MODE_REAL_16, XED_ADDRESS_WIDTH_16b);
}

//Clear the value of the flag, then update the value according to the bool status (fales = 0, true = 1) using only bitwise operators to speedup the operation
void CPU::updateFlag(const CPU::FLAGS_T& flag, const bool value)
{
	const unsigned int mask = (~0) * value;
	flags = (flags & ~flag) | (flag & mask);
}

void CPU::setFlags (const unsigned int requestedFlags)
{ flags |= requestedFlags; }

void CPU::toggleFlags (const unsigned int requestedFlags)
{ flags ^= requestedFlags; }

void CPU::clearFlags (const unsigned int requestedFlags)
{ flags &= (~requestedFlags); }

bool CPU::getFlagStatus (const CPU::FLAGS_T flag) const
{ return flags & (unsigned int) flag; }

void CPU::testCF (const unsigned int value, const bool wordSize)
{
	const unsigned int carryMask = ~(wordSize ? 0xFFFF : 0xFF);
	updateFlag(CARRY,value & carryMask);
}

void CPU::testPF (unsigned int value)
{
	unsigned int count = 1;

	while (value & 0xFF)
	{
		count += value & 1;
		value >>= 1;
	}
	updateFlag(PARRITY, count & 1);
}

//TODO: need to be tested
void CPU::testAF (const unsigned int value)
{ updateFlag(A_CARRY, value > (~0b111)); }

void CPU::testZF (const unsigned int value)
{ updateFlag(ZERRO,value == 0); }

void CPU::testSF (const unsigned int value)
{ updateFlag(SIGN, value & (1 << (sizeof(unsigned int) - 1))); }

void CPU::testOF (const unsigned int value, const bool wordSize)
{
	const bool newFlagValue = wordSize ? (value & (~0xFFFF)) : (value & (~0xFF));
	updateFlag(OVER,newFlagValue);
}

void CPU::updateStatusFlags (const unsigned int value, const bool wordSize)
{
	testCF(value, wordSize);
	testPF(value);
	testAF(value);
	testZF(value);
	testSF(value);
	testOF(value, wordSize);
}

static void printRegisters(const CPU& _cpu)
{
#if defined(SEE_REGS) ||  defined(SEE_ALL)
	std::printf("CS: %#6.4x   DS: %#6.4x   ES: %#6.4x   SS: %#6.4x\n",cpu.cs,cpu.ds,cpu.es,cpu.ss);
	std::printf("AX: %#6.4x   bx: %#6.4x   CX: %#6.4x   DX: %#6.4x\n",cpu.ax,cpu.bx,cpu.cx,cpu.dx);
	std::printf("SI: %#6.4x   DI: %#6.4x   BP: %#6.4x   SP: %#6.4x\n\n",cpu.si,cpu.di,cpu.bp,cpu.sp);
#endif
}

static void printFlags(const CPU& _cpu)
{
#if defined(SEE_FLAGS) || defined(SEE_ALL)
	std::cout << ((cpu.flags & CPU::CARRY) ? "CF" : "cf") << "  " << ((cpu.flags & CPU::PARRITY) ? "PF" : "pf") << "  " << ((cpu.flags & CPU::A_CARRY) ? "AF" : "af") << "  " << ((cpu.flags & CPU::ZERRO) ? "ZF" : "zf") << "  " << ((cpu.flags & CPU::SIGN) ? "SF" : "sf") << "  " << ((cpu.flags & CPU::TRAP) ? "TF" : "tf") << "  " << ((cpu.flags & CPU::INTF) ? "IF" : "if") << "  " << ((cpu.flags & CPU::DIR) ? "DF" : "df") << "  " << ((cpu.flags & CPU::OVER) ? "OF" : "of") << std::endl;
#endif
}

unsigned int CPU::genAddress (const uint16_t base, const uint16_t offset) const
{ return (base << 4) + offset; }

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
		return ax;

	case XED_REG_BX:
		return bx;

	case XED_REG_CX:
		return cx;

	case XED_REG_DX:
		return dx;

	case XED_REG_AH:
		return ah;

	case XED_REG_BH:
		return bh;

	case XED_REG_CH:
		return ch;

	case XED_REG_DH:
		return dh;

	case XED_REG_AL:
		return al;

	case XED_REG_BL:
		return bl;

	case XED_REG_CL:
		return cl;

	case XED_REG_DL:
		return dl;

	case XED_REG_SI:
		return si;

	case XED_REG_DI:
		return di;

	case XED_REG_BP:
		return bp;

	case XED_REG_SP:
		return sp;

	case XED_REG_CS:
		return cs;

	case XED_REG_DS:
		return ds;

	case XED_REG_ES:
		return es;

	case XED_REG_SS:
		return ss;
	}

	// Should never be reached but here to silent compiler warning
	// TODO: launch an exception here ? (probably yes, it is not ok if the program goes here)
	return 0;
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
			ERROR("Interrupt type not handled. Program quit\n");
			*(volatile unsigned int*)0 = 4;//Force error emission (by writing to memory address 0) if an interrupt type is not handled
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
	else if (INT3)// General int n interrupt
	{
		INT3 = false;
		INTERRUPT_SEQUENCE_CLOCK_COUNT = 52;
		INTERRUPT_VECTOR = 3;
	}
	else if (DIVIDE)
	{
		DIVIDE = false;
		INTERRUPT_SEQUENCE_CLOCK_COUNT = 51;//Guessed TODO: search the clock count for now I assume it is equivalent to int 0
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
	else if (INTR)// intr line asserted
	{
		INTR = false;
		if (!cpu.getFlagStatus(CPU::FLAGS_T::INTF))
		{
			debug<DEBUG_LEVEL_MAX>("INTEL 8088: INTERRUPT: interrupt request while IF is disabled");
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
	push(flags);
	cpu.eu.farCall(biu.readWord(INTERRUPT_VECTOR*4 + 2), biu.readWord(INTERRUPT_VECTOR*4));
	#ifdef DEBUG_BUILD
	printf("Interrupt (%#2x) data saved: %#4x:%#4x\n",INTERRUPT_VECTOR,cs,ip);
	#endif
	TEMP_TF = getFlagStatus(CPU::FLAGS_T::TRAP);
	clearFlags(CPU::FLAGS_T::INTF | CPU::FLAGS_T::TRAP);

	#ifdef DEBUG_BUILD
	printf("New cs:ip from %#x: %#4x:%#4x\n",INTERRUPT_VECTOR*4,cs,ip);
	#endif

	return NMI || TEMP_TF;
}

void CPU::iretDelay(void) { IRET_DELAY = true; }

bool CPU::clock()
{
	if (!CPU_HLT)
		cpu.biu.clock();
	const bool instructionExecuted = cpu.eu.clock();

	if (instructionExecuted)
	{
		biu.instructionBufferQueuePop(cpu.eu.instructionLength);
		instructionExecutedCount += 1;
		#if defined(SEE_ALL) || defined(SEE_REGS)
			printRegisters();
		#endif

		#if defined(SEE_ALL) || defined(SEE_FLAGS)
			printFlags();
		#endif

		//TODO: This might optimization by using bit mask to tell which interrupt is triggered
		const bool shouldInterrupt = (!IRET_DELAY) && (INTR || INTN || INTO || INT3 || DIVIDE || NMI || cpu.getFlagStatus(CPU::FLAGS_T::TRAP));
		if (shouldInterrupt)
			handleInterrupts();
		IRET_DELAY = false;
	}

	eu.updateClockFunction();
	biu.updateClockFunction();

	return instructionExecuted;
}

void CPU::hlt(void) { CPU_HLT = true; }

void CPU::push (const uint16_t data)
{
	cpu.sp -= 2;
	cpu.biu.writeWord(cpu.genAddress(cpu.ss,cpu.sp), data);
}

uint16_t CPU::pop (void)
{
	const uint16_t ret = cpu.biu.readWord(cpu.genAddress(cpu.ss,cpu.sp));
	cpu.sp += 2;
	return ret;
}

void CPU::write_reg(const xed_reg_enum_t reg, const unsigned int data)
{
	switch (reg)
	{
	case XED_REG_AX:
		ax = data;
		break;

	case XED_REG_BX:
		bx = data;
		break;

	case XED_REG_CX:
		cx = data;
		break;

	case XED_REG_DX:
		dx = data;
		break;

	case XED_REG_AH:
		ah = data;
		break;

	case XED_REG_BH:
		bh = data;
		break;

	case XED_REG_CH:
		ch = data;
		break;

	case XED_REG_DH:
		dh = data;
		break;

	case XED_REG_AL:
		al = data;
		break;

	case XED_REG_BL:
		bl = data;
		break;

	case XED_REG_CL:
		cl = data;
		break;

	case XED_REG_DL:
		dl = data;
		break;

	case XED_REG_SI:
		si = data;
		break;

	case XED_REG_DI:
		di = data;
		break;

	case XED_REG_BP:
		bp = data;
		break;

	case XED_REG_SP:
		sp = data;
		break;

	case XED_REG_CS:
		cs = data;
		break;

	case XED_REG_DS:
		ds = data;
		break;

	case XED_REG_ES:
		es = data;
		break;

	case XED_REG_SS:
		ss = data;
		break;
	}
}
