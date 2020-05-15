#include "arch.hpp" //arch.hpp includes 8086.hpp

CPU::CPU(RAM &ram, PORTS &ports, E5150::Arch& arch) : m_flags(0), m_ip(0), m_regs(), m_ram(ram), hlt(false), m_ports(ports),
								   intr(false), nmi(false), intr_v(0), m_gregs(), interrupt_enable(true), m_arch(arch),m_clockCountDown(0)
{
	std::cout << xed_get_copyright() << std::endl;

	m_regs[CS] = 0xF000;
	m_regs[SS] = 0xEF0;
	m_regs[SP] = 0xFF;
	m_ip = 0xFFF0;
	m_flags = 0x02;

	/* initialisation de xed */
	xed_tables_init();
	xed_decoded_inst_zero(&m_decoded_inst);
	xed_decoded_inst_set_mode(&m_decoded_inst, XED_MACHINE_MODE_REAL_16, XED_ADDRESS_WIDTH_16b);
}

void CPU::request_nmi (void)
{
	nmi = true;
	request_intr(2);
}

void CPU::request_intr (const uint8_t vector)
{ intr_v = vector; }

unsigned int CPU::gen_address (const reg_t base, const uint16_t offset) const
{ return (base << 4) + offset; }

unsigned int CPU::gen_address (const reg_t base, const xed_reg_enum_t offset) const 
{ return gen_address(base, read_reg(offset)); }

unsigned int CPU::gen_address (const xed_reg_enum_t segment, const uint16_t offset) const 
{ return gen_address(read_reg(segment), offset); }

unsigned int CPU::gen_address (const xed_reg_enum_t segment, const xed_reg_enum_t offset) const 
{return gen_address(read_reg(segment), read_reg(offset));}

/**
 * This function compute the effective address for a memory operand and add the corresponding number of 
 * instructions cycles. The instruction cycles are îcked up from https://zsmith.co/intel.php#ea:
 * 1.  disp: mod = 0b00 and rm = 0b110								+6
 * 2.  (BX,BP,SI,DI): mod = 0b00 and rm != 0b110 and rm = 0b1xx		+5
 * 3.  disp + (BX,BP,SI,DI): mod = 0b10 and rm = 0b1xx				+9
 * 4.1 (BP+DI, BX+SI): mod = 0b00 and rm = 0b01x					+7
 * 4.2 (BP+SI, BX+DI): mod = 0b00 and rm = 0b00x					+8
 * 5.1 disp + (BP+DI, BX+SI) +-> same as precedet with mod = 0b10	+11
 * 5.2 disp + (BP+SI, BX+DI) +										+12
 * 
 * word operands at odd addresses	+4
 * segment override					+2
 */
unsigned int CPU::genEA (const xed_operand_enum_t op_name) 
{
	const unsigned int modrm = xed_decoded_inst_get_modrm(&m_decoded_inst);
	const unsigned int mod = (modrm & 0b11000000) >> 6;
	const unsigned int rm = modrm & 0b111;

	if (mod == 0b00)
	{
		//1. disp: mod == 0b00 and rm 0b110
		if (rm == 0b110)
			m_clockCountDown += 6;
		else
		{
			//2. (base,index) mod = 0b00 and rm = 0b1xx and rm != 0b110
			if (rm & 0b100)
				m_clockCountDown += 5;
			//4.1/4.2 base + index mod = 0b00 and rm = 0b01x/0b00x
			else
				m_clockCountDown += (rm & 0b10) ? 7 : 8;
		}
	}
	//mod = 0b10
	else
	{
		//3. disp + (base,index): mod = 0b10 rm = 0b1xx
		if (rm & 0b100)
			m_clockCountDown += 9;
		//5.1/5.2 base + index + disp: mod = 0b10 rm = 0b01x/0b00x
		else
			m_clockCountDown += (rm & 0b10) ? 11 : 12;
	}

	const unsigned int address = gen_address(xed_decoded_inst_get_seg_reg(&m_decoded_inst,0), xed_decoded_inst_get_memory_displacement(&m_decoded_inst,0));

	if (xed_decoded_inst_get_memory_operand_length(&m_decoded_inst, 0) == 2)
		m_clockCountDown += (address & 0b1) ? 4 : 0;
	
	if (xed_operand_values_has_segment_prefix(&m_decoded_inst))
		m_clockCountDown += 2;
	
	return address;

}

uint8_t  CPU::readByte (const unsigned int addr) const 
{ return m_ram.read(addr); }

uint16_t CPU::readWord (const unsigned int addr) const 
{ return (uint16_t)m_ram.read(addr) | (uint16_t)(m_ram.read(addr + 1) << 8); }

void CPU::writeByte (const unsigned int addr, const uint8_t  data) 
{ m_ram.write(addr, data); }

void CPU::writeWord (const unsigned int addr, const uint16_t data) 
{ m_ram.write(addr, (uint8_t)data); m_ram.write(addr + 1, (uint8_t)(data >> 8)); }

void CPU::push (const uint16_t data)
{ m_regs[SP] -= 2; writeWord(gen_address(m_regs[SS], m_regs[SP]), data); }

uint16_t CPU::pop (void)
{
	const uint16_t ret = readWord(gen_address(m_regs[SS], m_regs[SP])); m_regs[SP] += 2;
	return ret;
}

void CPU::far_call (const reg_t seg, const uint16_t offset)
{
	push(m_regs[CS]);
	push(m_ip);

	m_regs[CS] = seg;
	m_ip	   = offset;
}

void CPU::far_ret (void)
{
	m_ip = pop();
	m_regs[CS] = pop();
}

void CPU::interrupt (void)
{
	push(m_flags);
	clearFlags(INTF);
	far_call(readWord(gen_address(0, 4 * intr_v + 2)), readWord(gen_address(0, 4 * intr_v)));
	hlt = false;
}

void CPU::setFlags (const unsigned int flags)
{ m_flags |= flags; }

void CPU::clearFlags (const unsigned int flags)
{ m_flags &= (~flags); }

bool CPU::get_flag_status (const unsigned int flag)
{ return (m_flags & (unsigned int) flag) == flag; }

void CPU::testCF (const unsigned int value, const bool byte)
{
	if (byte)
	{
		if (value > (uint8_t)0xFF || ((signed)value < (int8_t)0x80))
			setFlags(CARRY);
		else
			clearFlags(CARRY);
	}
	else
	{
		if (value > (uint8_t)0xFFFF || ((signed)value < (int8_t)0x8000))
			setFlags(CARRY);
		else
			clearFlags(CARRY);
	}
}

void CPU::testPF (const unsigned int value)
{
	unsigned int test = 1 << 7;
	unsigned int count = 0;

	while (test)
	{
		if (test & value)
			++count;
		
		test >>= 1;
	}

	if (count & 1)
		setFlags(PARRITY);
	else
		clearFlags(PARRITY);
}

void CPU::testAF (const unsigned int value)
{
	//TODO: need to be tested
	if (value > (~0b111))
		setFlags(A_CARRY);
	else
		clearFlags(A_CARRY);
}

void CPU::testZF (const unsigned int value)
{
	if (value == 0)
		setFlags(ZERRO);
	else
		clearFlags(ZERRO);
}

void CPU::testSF (const unsigned int value)
{
	if (value & 0x8000)//bit 15 set
		setFlags(SIGN);
	else
		clearFlags(SIGN);
}

void CPU::testOF (const unsigned int value, const bool byte)
{
	if (byte)
	{
		if (value & (~0xFF))
			setFlags(OVER);
		else
			clearFlags(OVER);
	}
	else
	{
		if (value & (~0xFFFF))
			setFlags(OVER);
		else
			clearFlags(OVER);
	}
}

void CPU::updateStatusFlags (const unsigned int value, const bool byte)
{
	testCF(value, byte);
	testPF(value);
	testAF(value);
	testZF(value);
	testSF(value);
	testOF(value, byte);
}

void CPU::printRegisters() const
{
	std::cout << std::hex << std::showbase << "CS: " << m_regs[CS] << "  DS: " << m_regs[DS] << "   ES: " << m_regs[ES] << "   SS: " << m_regs[SS] << std::endl;
	std::cout << "AX: " << m_gregs[AX].x << "  BX: " << m_gregs[BX].x << "   CX: " << m_gregs[CX].x << "   DX: " << m_gregs[DX].x << std::endl;
	std::cout << "SI: " << m_regs[SI] << "  DI: " << m_regs[DI] << "   BP: " << m_regs[BP] << "   SP: " << m_regs[SP] << '\n'
			  << std::noshowbase << std::dec << std::endl;
}

void CPU::printFlags() const
{
	std::cout << ((m_flags & CARRY) ? "CF" : "cf") << "  " << ((m_flags & PARRITY) ? "PF" : "pf") << "  " << ((m_flags & A_CARRY) ? "AF" : "af") << "  " << ((m_flags & ZERRO) ? "ZF" : "zf") << "  " << ((m_flags & SIGN) ? "SF" : "sf") << "  " << ((m_flags & TRAP) ? "TF" : "tf") << "  " << ((m_flags & INTF) ? "IF" : "if") << "  " << ((m_flags & DIR) ? "DF" : "df") << "  " << ((m_flags & OVER) ? "OF" : "of") << std::endl;
}

void CPU::printCurrentInstruction() const
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&m_decoded_inst);
	std::cout << std::hex << std::showbase << m_regs[CS] << ":" << m_ip << " (" << gen_address(m_regs[CS], m_ip) << ")" << std::dec << std::endl;
	std::cout << xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(&m_decoded_inst)) << " : length = " << xed_decoded_inst_get_length(&m_decoded_inst) << std::endl;

	for (unsigned int i = 0; i < xed_decoded_inst_noperands(&m_decoded_inst); ++i)
	{
		const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, i));
		const xed_operand_visibility_enum_t op_vis = xed_operand_operand_visibility(xed_inst_operand(inst, i));

		if (op_vis == XED_OPVIS_SUPPRESSED)
			std::cout << "( ";

		std::cout << "operand" << i << ": ";

		switch (op_name)
		{
		case XED_OPERAND_RELBR:
			std::cout << xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
			break;

		case XED_OPERAND_PTR:
			std::cout << xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
			break;

		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
		case XED_OPERAND_REG2:
			std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_reg(&m_decoded_inst, op_name));
			break;

		case XED_OPERAND_IMM0:
		case XED_OPERAND_IMM1:
			std::cout << xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst);
			break;

		case XED_OPERAND_MEM0:
			std::cout << ((xed_decoded_inst_get_memory_operand_length(&m_decoded_inst, 0) == 1) ? "BYTE" : "WORD") << " ";
			std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_seg_reg(&m_decoded_inst, 0)) << ":[";
			std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_index_reg(&m_decoded_inst, 0)) << "(index) + ";
			std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_base_reg(&m_decoded_inst, 0)) << "(base) + ";
			std::cout << xed_decoded_inst_get_memory_displacement(&m_decoded_inst, 0);
			std::cout << "]";
			break;

		default:
			break;
		}

		std::cout << " (" << xed_operand_enum_t2str(op_name) << ") ";

		if (op_vis == XED_OPVIS_SUPPRESSED)
			std::cout << ")";

		std::cout << std::endl;
	}
}

bool CPU::execNonControlTransferInstruction()
{
	bool hasExecutedInstruction = true;

	//TODO: optimize this switch
	switch (xed_decoded_inst_get_iclass(&m_decoded_inst))
	{
	case XED_ICLASS_MOV:
		MOV();
		break;

	case XED_ICLASS_PUSH:
		PUSH();
		break;

	case XED_ICLASS_POP:
		POP();
		break;

	case XED_ICLASS_ADD:
		ADD();
		break;

	case XED_ICLASS_INC:
		INC();
		break;

	case XED_ICLASS_SUB:
		SUB();
		break;

	case XED_ICLASS_DEC:
		DEC();
		break;

	case XED_ICLASS_NEG_LOCK:
	case XED_ICLASS_NEG:
		NEG();
		break;

	case XED_ICLASS_CMP:
		CMP();
		break;

	case XED_ICLASS_MUL:
		MUL();
		break;

	case XED_ICLASS_IMUL:
		IMUL();
		break;

	case XED_ICLASS_DIV:
		DIV();
		break;

	case XED_ICLASS_IDIV:
		IDIV();
		break;

	case XED_ICLASS_XCHG:
		XCHG();
		break;

	case XED_ICLASS_NOT:
		NOT();
		break;

	case XED_ICLASS_IN:
		IN();
		break;

	case XED_ICLASS_OUT:
		OUT();
		break;

	case XED_ICLASS_XLAT:
		XLAT();
		break;

	case XED_ICLASS_LEA:
		LEA();
		break;

	case XED_ICLASS_LDS:
		LDS();
		break;

	case XED_ICLASS_LES:
		LES();
		break;

	case XED_ICLASS_LAHF:
		LAHF();
		break;

	case XED_ICLASS_SAHF:
		SAHF();
		break;

	case XED_ICLASS_PUSHF:
		PUSHF();
		break;

	case XED_ICLASS_POPF:
		POPF();
		break;

	case XED_ICLASS_CLC:
		CLC();
		break;

	case XED_ICLASS_STC:
		STC();
		break;

	case XED_ICLASS_CLI:
		CLI();
		break;

	case XED_ICLASS_STI:
		STI();
		break;

	case XED_ICLASS_CLD:
		CLD();
		break;

	case XED_ICLASS_STD:
		STD();
		break;

	case XED_ICLASS_HLT:
		HLT();
		break;
	
	default:
		hasExecutedInstruction = false;
	}

	m_ip += xed_decoded_inst_get_length(&m_decoded_inst);

	return hasExecutedInstruction;
}

void CPU::execControlTransferInstruction()
{
	/* Control transfert */
	//TODO: Implement Jcc instructions
	switch (xed_decoded_inst_get_iclass(&m_decoded_inst))
	{
	case XED_ICLASS_CALL_NEAR:
		NEAR_CALL();
		break;

	case XED_ICLASS_CALL_FAR:
		FAR_CALL();
		break;

	case XED_ICLASS_JMP:
		NEAR_JMP();
		break;

	case XED_ICLASS_JMP_FAR:
		FAR_JMP();
		break;

	case XED_ICLASS_RET_NEAR:
		NEAR_RET();
		break;

	case XED_ICLASS_RET_FAR:
		FAR_RET();
		break;

	case XED_ICLASS_JZ:
		JZ();
		break;

	case XED_ICLASS_JL:
		JL();
		break;

	case XED_ICLASS_JLE:
		JLE();
		break;

	case XED_ICLASS_JNZ:
		JNZ();
		break;

	case XED_ICLASS_JNL:
		JNL();
		break;

	case XED_ICLASS_JNLE:
		JNLE();
		break;

	case XED_ICLASS_LOOP:
		LOOP();
		break;

	case XED_ICLASS_JCXZ:
		JCXZ();
		break;

	case XED_ICLASS_INT:
		INT();
		break;

	case XED_ICLASS_IRET:
		IRET();
		break;
	}
}

void CPU::simulate()
{
	if (m_clockCountDown == 0)
	{
		if (!hlt)
		{
			xed_decoded_inst_zero_keep_mode(&m_decoded_inst);
			xed_decode(&m_decoded_inst, m_ram.m_ram + gen_address(m_regs[CS], m_ip), 16);

#if defined(SEE_REGS) ||  defined(SEE_ALL)
			printRegisters();
#endif

#if defined(SEE_FLAGS) || defined(SEE_ALL)
			printFlags();
#endif

#if defined(SEE_CURRENT_INST) || defined(SEE_ALL)
			printCurrentInstruction();
#endif
#ifdef STOP_AT_END
			PAUSE
#endif

			if (!execNonControlTransferInstruction())
				execControlTransferInstruction();
		}
#ifdef STOP_AT_END
		else
		{
			std::cout << "CPU HALTED !" << std::endl;
			PAUSE
		}
#endif
	}
	else
	{
		#ifdef CLOCK_DEBUG
			std::cout << "clock: " <<
		#endif
		m_clockCountDown--
		#ifdef CLOCK_DEBUG
			<< std::endl
		#endif
		;
	}

	if (nmi)
	{
		interrupt();
		nmi = false;
	}
	else if (intr)
	{
		if (get_flag_status(INTF))
			interrupt();
		intr = false;
	}
}

void CPU::write_reg(const xed_reg_enum_t reg, const unsigned int data)
{
	switch (reg)
	{
	case XED_REG_AX:
		m_gregs[AX].x = data;
		break;

	case XED_REG_BX:
		m_gregs[BX].x = data;
		break;

	case XED_REG_CX:
		m_gregs[CX].x = data;
		break;

	case XED_REG_DX:
		m_gregs[DX].x = data;
		break;

	case XED_REG_AH:
		m_gregs[AX].h = data;
		break;

	case XED_REG_BH:
		m_gregs[BX].h = data;
		break;

	case XED_REG_CH:
		m_gregs[CX].h = data;
		break;

	case XED_REG_DH:
		m_gregs[DX].h = data;
		break;

	case XED_REG_AL:
		m_gregs[AX].l = data;
		break;

	case XED_REG_BL:
		m_gregs[BX].l = data;
		break;

	case XED_REG_CL:
		m_gregs[CX].l = data;
		break;

	case XED_REG_DL:
		m_gregs[DX].l = data;
		break;

	case XED_REG_SI:
		m_regs[SI] = data;
		break;

	case XED_REG_DI:
		m_regs[DI] = data;
		break;

	case XED_REG_BP:
		m_regs[BP] = data;
		break;

	case XED_REG_SP:
		m_regs[SP] = data;
		break;

	case XED_REG_CS:
		m_regs[CS] = data;
		break;

	case XED_REG_DS:
		m_regs[DS] = data;
		break;

	case XED_REG_ES:
		m_regs[ES] = data;
		break;

	case XED_REG_SS:
		m_regs[SS] = data;
		break;
	}
}

uint16_t CPU::read_reg(const xed_reg_enum_t reg) const
{
	switch (reg)
	{
	case XED_REG_AX:
		return m_gregs[AX].x;

	case XED_REG_BX:
		return m_gregs[BX].x;

	case XED_REG_CX:
		return m_gregs[CX].x;

	case XED_REG_DX:
		return m_gregs[DX].x;

	case XED_REG_AH:
		return m_gregs[AX].h;

	case XED_REG_BH:
		return m_gregs[BX].h;

	case XED_REG_CH:
		return m_gregs[CX].h;

	case XED_REG_DH:
		return m_gregs[DX].h;

	case XED_REG_AL:
		return m_gregs[AX].l;

	case XED_REG_BL:
		return m_gregs[BX].l;

	case XED_REG_CL:
		return m_gregs[CX].l;

	case XED_REG_DL:
		return m_gregs[DX].l;

	case XED_REG_SI:
		return m_regs[SI];

	case XED_REG_DI:
		return m_regs[DI];

	case XED_REG_BP:
		return m_regs[BP];

	case XED_REG_SP:
		return m_regs[SP];

	case XED_REG_CS:
		return m_regs[CS];

	case XED_REG_DS:
		return m_regs[DS];

	case XED_REG_ES:
		return m_regs[ES];

	case XED_REG_SS:
		return m_regs[SS];
	}

	// Should never be reached but here to silent compiler warning
	// TODO: launch an exception here ? (probably here, it is not ok the program goes here)
	return 0;
}
