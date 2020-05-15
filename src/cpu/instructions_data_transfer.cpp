#include "8086.hpp"

void CPU::MOV()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&m_decoded_inst);

	xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 1));

	uint16_t move_v = 0;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			move_v = read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name));
			break;

		case XED_OPERAND_REG1:
			move_v = read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name));
			break;
		
		case XED_OPERAND_IMM0:
			move_v = xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst);
			break;

		case XED_OPERAND_MEM0:
			move_v = readWord(genEA(op_name));
			break;
	}

	op_name = xed_operand_name(xed_inst_operand(inst, 0));

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			write_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name), move_v);
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int write_addr = genEA(op_name);

			if (xed_decoded_inst_get_memory_operand_length(&m_decoded_inst, 0) == 1)
				writeByte(write_addr, (uint8_t)move_v);
			else
				writeWord(write_addr, move_v);
			
			break;
		}
	}
}

void CPU::PUSH (void)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			push(read_reg(xed_decoded_inst_get_reg(&m_decoded_inst,op_name)));
			break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int data_to_push_addr = genEA(op_name);
			push(readWord(data_to_push_addr));
		}
		break;
	}
}

void CPU::POP (void)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t tmp_reg = xed_decoded_inst_get_reg(&m_decoded_inst, op_name);

			write_reg(tmp_reg, pop());

			if (tmp_reg == XED_REG_SP)
				m_regs[SP] += 2;

			break;
		}

	}
}

void CPU::XCHG()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&m_decoded_inst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));
	// The second operand is always a register
	const xed_reg_enum_t register_xchg = xed_decoded_inst_get_reg(&m_decoded_inst, xed_operand_name(xed_inst_operand(inst, 1)));

	const uint16_t value1 = read_reg(register_xchg);
	uint16_t value2;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			write_reg(register_xchg, read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name)));
			write_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name), value1);
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = genEA(op_name);
			const unsigned int value2 = readWord(addr);
			write_reg(register_xchg, value2);

			if (xed_decoded_inst_get_memory_operand_length(&m_decoded_inst, 0) == 1)
				writeByte(addr, value1);
			else
				writeWord(addr, value1);
		}
	}

}

void CPU::IN()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&m_decoded_inst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));

	uint16_t iaddr = 0;

	if (op_name == XED_OPERAND_IMM0)
		iaddr = (uint16_t)xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst);
	else 
		iaddr = m_gregs[DX].x;

	m_gregs[AX].l = m_ports.read(iaddr);

	if (xed_decoded_inst_get_reg(&m_decoded_inst, xed_operand_name(xed_inst_operand(inst, 1))) == XED_REG_AX)
		m_gregs[AX].h = m_ports.read(iaddr + 1);
}

void CPU::OUT()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&m_decoded_inst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));

	uint16_t oaddr = 0;

	if (op_name == XED_OPERAND_IMM0)
		oaddr = (uint16_t)xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst);
	else
		oaddr = m_gregs[DX].x;
		
	m_ports.write(oaddr, m_gregs[AX].l);

	if (xed_decoded_inst_get_reg(&m_decoded_inst, xed_operand_name(xed_inst_operand(inst, 1))) == XED_REG_AX)
		m_ports.write(oaddr + 1, m_gregs[AX].h);
}

void CPU::XLAT  (void)
{ m_gregs[AX].l = readByte(gen_address(m_regs[DS], m_gregs[BX].x + m_gregs[AX].l)); }

void CPU::LEA()
{
	const xed_operand_enum_t op_name0 = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 1));

	write_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name0), genEA(op_name1));
}

void CPU::LDS()
{
	//TODO: Implement LDS
}

void CPU::LES()
{
	//TODO: Implement LES
}

void CPU::LAHF  (void) 
{ m_gregs[AX].h = (m_flags & 0b11010101)|0b10; }

void CPU::SAHF  (void) 
{ setFlags(m_gregs[AX].h & (0b11010101)); }

void CPU::PUSHF (void) 
{ push(m_flags); }

void CPU::POPF  (void) 
{ m_flags = pop(); }
