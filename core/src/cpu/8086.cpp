#include "arch.hpp"
#include "8086.hpp"
#include "instructions.hpp"

CPU::CPU() : hlt(false), intr(false), nmi(false), intr_v(0),interrupt_enable(true),clockCountDown(0),
							instructionExecutedCount(0)
{
	std::cout << xed_get_copyright() << std::endl;

	cs = 0xF000;
	ss = 0xEF0;
	sp = 0xFF;
	ip = 0xFFF0;
	flags = 0x02;
	addressBus = genAddress(cs,ip);

	/* initialisation de xed */
	xed_tables_init();
	xed_decoded_inst_zero(&decodedInst);
	xed_decoded_inst_set_mode(&decodedInst, XED_MACHINE_MODE_REAL_16, XED_ADDRESS_WIDTH_16b);
}

void CPU::requestNmi (void)
{
	nmi = true;
	requestIntr(2);
}

void CPU::requestIntr (const uint8_t vector)
{ intr_v = vector; intr = true; }

void CPU::interrupt (const bool isNMI)
{
	E5150::Util::_stop = true;
	const unsigned int interruptVector = isNMI ? 2 : intr_v;
	cpu.eu.push(flags);
	clearFlags(INTF);
	cpu.eu.farCall(cpu.biu.readWord(genAddress(0, 4 * interruptVector + 2)), cpu.biu.readWord(genAddress(0, 4 * interruptVector)));
	hlt = false;
}

//Clear the value of the flag, then update the value according to the bool status (fales = 0, true = 1) using only bitwise operators to speedup the operation
void CPU::updateFlag(const CPU::FLAGS_T& flag, const bool value)
{ flags = (flags & ~flag) | ((unsigned)value << flag); }

void CPU::setFlags (const unsigned int requestedFlags)
{ flags |= requestedFlags; }

void CPU::toggleFlags (const unsigned int requestedFlags)
{ flags ^= requestedFlags; }

void CPU::clearFlags (const unsigned int requestedFlags)
{ flags &= (~requestedFlags); }

bool CPU::getFlagStatus (const CPU::FLAGS_T flag) const
{ return flags & (unsigned int) flag; }

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
unsigned int CPU::genEA()
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

	const unsigned int address = genAddress(xed_decoded_inst_get_seg_reg(&decodedInst,0), xed_decoded_inst_get_memory_displacement(&decodedInst,0));
	
	if (xed_operand_values_has_segment_prefix(&decodedInst))
		clockNeeded += 2;
	
	return address;
}

void CPU::testCF (const unsigned int value, const bool wordSize)
{
	const bool newFlagValue = wordSize ? (value > (uint8_t)0xFFFF || ((signed)value < (int8_t)0x8000)): 
										 (value > (uint8_t)0x00FF || ((signed)value < (int8_t)0x0080));
	updateFlag(CARRY,newFlagValue);
}

void CPU::testPF (unsigned int value)
{
	unsigned int count = 0;

	while (value)
	{
		if (value & 0b1)
			++count;
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
bool CPU::isHalted (void) const { return hlt; }

static bool cpuCanProcessClock (const CPU* _cpu)
{ /*return ((cpu->m_clockCountDown == 0) && (!cpu->hlt));*/ return true; }

bool CPU::decode()
{/*
	const bool canProcessClock = cpuCanProcessClock(this);
	if (canProcessClock)
	{
		xed_decoded_inst_zero_keep_mode(&m_decoded_inst);
		xed_decode(&m_decoded_inst, m_ram.m_ram + gen_address(m_regs[CS],m_ip),16);
	}

	return canProcessClock;
	*/
	return true;
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

bool CPU::clock()
{
	if (!hlt)
		cpu.biu.clock();
	const bool instructionExecuted = cpu.eu.clock();

	cpu.biu.clock = cpu.biu.nextClockFunction;
	cpu.eu.clock = cpu.eu.nextClockFunction;
	return instructionExecuted;
#if 0
	if (clockCountDown != 0)
	{
		--clockCountDown;
		return;
	}

	if (!hlt)
	{
		xed_decoded_inst_zero_keep_mode(&decodedInst);
		xed_decode(&decodedInst, ram.m_ram + genAddress(cs,ip), 16);

	#if defined(STOP_AT_END) || defined(CLOCK_DEBUG)
		if (E5150::Util::_stop)
		{
			printRegisters(*this);
			printFlags(*this);
			printCurrentInstruction(*this);
			clockWait();
		}
	#endif

		if (!execNonControlTransferInstruction(*this))
			execControlTransferInstruction(*this);
	}

	if (intr)
	{
		intr = false;

		if (nmi)
		{
			interrupt(true);
			nmi = false;
		}
		else
		{
			if (getFlagStatus(INTF))
				interrupt();
			else
				debug<6>("CPU: INTERRUPT: interrupt request while IF is disabled");
		}
	}
#endif
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
