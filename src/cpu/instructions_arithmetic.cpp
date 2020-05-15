#include "8086.hpp"

void CPU::ADD()
{
	const unsigned int iform = xed_decoded_inst_get_iform_enum(&m_decoded_inst);
	const bool cond = ((iform >= 33) && (iform <= 38)) || ((iform >= 44) && (iform <= 46));
	const xed_inst_t* inst = xed_decoded_inst_inst(&m_decoded_inst);
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst, 0));
	const xed_operand_enum_t op_name2 = xed_operand_name(xed_inst_operand(inst, 1));

	unsigned int value1 = 0;

	switch (op_name2)
	{
		case XED_OPERAND_IMM0:
			value1 += xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst);
			break;
		
		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			value1 += read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name2));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = genEA(op_name2);
			value1 += cond ? readByte(addr) : readWord(addr);
		} break;
	}

	switch (op_name1)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&m_decoded_inst, op_name1);
			value1 += read_reg(reg);
			write_reg(reg, value1);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = genEA(op_name1);
			value1 += cond ? readByte(addr) : readWord(addr);

			if (cond)
				writeByte(addr, value1);
			else
				writeWord(addr, value1);
		} break;
	}

	updateStatusFlags(value1, cond ? 1 : 2);
}

void CPU::INC()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));

	const unsigned int iform = xed_decoded_inst_get_iform_enum(&m_decoded_inst);
	const bool cond = (iform == XED_IFORM_INC_GPR8) || (iform == XED_IFORM_INC_MEMb) || (iform == XED_IFORM_INC_LOCK_MEMb);

	unsigned int value1;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&m_decoded_inst, op_name);
			value1 = read_reg(reg) + 1;
			write_reg(reg, value1);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = genEA(op_name);
			value1 = (cond ? readByte(addr) : readWord(addr)) + 1;

			if (cond)
				writeByte(addr, value1);
			else
				writeWord(addr, value1);
		} break;
	}

	updateStatusFlags(value1, cond ? 1 : 2);
}

void CPU::SUB()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&m_decoded_inst);
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst, 0));
	const xed_operand_enum_t op_name2 = xed_operand_name(xed_inst_operand(inst, 1));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&m_decoded_inst);
	const bool cond = 	((iform >= XED_IFORM_SUB_AL_IMMb) && (iform >= XED_IFORM_SUB_GPR8_MEMb)) ||
					  	((iform >= XED_IFORM_SUB_MEMb_GPR8) && (iform <= XED_IFORM_SUB_MEMb_IMMb_82r5));

	unsigned int value1;

	switch (op_name2)
	{
		case XED_OPERAND_IMM0:
			value1 -= xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst);
			break;
		
		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			value1 -= read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name2));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = genEA(op_name2);
			value1 -= cond ? readByte(addr) : readWord(addr);
		} break;
	}

	switch (op_name1)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&m_decoded_inst, op_name1);

			value1 += read_reg(reg);
			write_reg(reg, value1);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = genEA(op_name1);
			value1 += cond ? readByte(addr) : readWord(addr);

			if (cond) writeByte(addr, value1); else writeWord(addr, value1);
		} break;
	}

	updateStatusFlags(value1, cond ? 1 : 2);
}

void CPU::DEC()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));
	const xed_iform_enum_t   iform   = xed_decoded_inst_get_iform_enum(&m_decoded_inst);
	const bool cond = (iform == XED_IFORM_DEC_GPR8) || (iform == XED_IFORM_DEC_MEMb) || (XED_IFORM_DEC_LOCK_MEMb);

	unsigned int value1;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&m_decoded_inst, op_name);

			value1 = read_reg(reg) - 1;
			write_reg(reg, value1);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = genEA(op_name);
			value1 = (cond ? readByte(addr) : readWord(addr)) - 1;

			if (cond)
				writeByte(addr, value1);
			else
				writeWord(addr, value1);
		} break;
	}

	updateStatusFlags(value1, cond ? 1 : 2);
}

void CPU::NEG()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&m_decoded_inst);
	const bool cond = (iform == XED_IFORM_NEG_GPR8) || (iform == XED_IFORM_NEG_MEMb) || (iform == XED_IFORM_NEG_LOCK_MEMb);

	unsigned int value1;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&m_decoded_inst, op_name);

			value1 = read_reg(reg);
			value1 *= -1;
			write_reg(reg, value1);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = genEA(op_name);

			value1 = cond ? readByte(addr) : readWord(addr);
			value1 *= -1;

			if (cond)
				writeByte(addr, value1);
			else
				writeWord(addr, value1);
		} break;
	}

	if (value1 == 0)
		clearFlags(CARRY);
	else
		setFlags(CARRY);
	
	updateStatusFlags(value1, cond ? 1: 2);
}

void CPU::CMP()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&m_decoded_inst);
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst, 0));
	const xed_operand_enum_t op_name2 = xed_operand_name(xed_inst_operand(inst, 1));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&m_decoded_inst);
	const bool cond = 	((iform >= XED_IFORM_CMP_AL_IMMb) && (iform <= XED_IFORM_CMP_GPR8_MEMb)) ||
						((iform >= XED_IFORM_CMP_MEMb_GPR8) && (iform <= XED_IFORM_CMP_MEMb_IMMb_82r7));

	unsigned int value1 = 0;

	switch (op_name1)
	{
		case XED_OPERAND_REG0:
			value1 += read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name1));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = genEA(op_name1);
			value1 += cond ? readByte(addr) : readWord(addr);
		} break;
	}

	switch (op_name2)
	{
		case XED_OPERAND_IMM0:
			value1 -= xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst);
			break;
			
		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			value1 -= read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name2));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = genEA(op_name2);
			value1 -= cond ? readByte(addr) : readWord(addr);
		} break;
	}

	updateStatusFlags(value1, cond ? 1 : 2);
}

void CPU::MUL()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&m_decoded_inst);
	const bool cond = (iform == XED_IFORM_MUL_GPR8) || (iform == XED_IFORM_MUL_MEMb);

	unsigned int value1 = 1;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			value1 *= read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name));
			break;
		
		case XED_OPERAND_MEM0:
			value1 *= cond ? readByte(genEA(op_name)) : readWord(genEA(op_name));
			break;
	}

	value1 *= cond ? m_gregs[AX].l : m_gregs[AX].x;

	m_gregs[AX].x = value1;

	if (!cond)
		m_gregs[DX].x = value1 >> 16;
	
	if (cond)
	{
		if (m_gregs[AX].h == 0)
			clearFlags(CARRY|OVER);
		else
			setFlags(CARRY|OVER);
	}
	else
	{
		if (m_gregs[DX].x == 0)
			clearFlags(CARRY|OVER);
		else
			setFlags(CARRY|OVER);
	}
}

void CPU::IMUL()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&m_decoded_inst);
	const bool cond = (iform == XED_IFORM_IMUL_GPR8) || (iform == XED_IFORM_IMUL_MEMb);

	int value1 = 1;

	switch(op_name)
	{
		case XED_OPERAND_REG0:
			value1 *= (signed)read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name));
			break;
		
		case XED_OPERAND_MEM0:
			value1 *= (signed)(cond ? readByte(genEA(op_name)) : readWord(genEA(op_name)));
			break;
	}

	value1 *= (signed)(cond ? m_gregs[AX].l : m_gregs[AX].x); 

	m_gregs[AX].x = value1;

	if (!cond)
		m_gregs[DX].x = value1 >> 16;
	
	if (cond)
	{
		if (m_gregs[AX].h == 0)
			clearFlags(CARRY|OVER);
		else
			setFlags(CARRY|OVER);
	}
	else
	{
		if (m_gregs[DX].x == 0)
			clearFlags(CARRY|OVER);
		else
			setFlags(CARRY|OVER);
	}
}

//TODO: what happens when dividing by zero ? Restart ?
void CPU::DIV()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&m_decoded_inst);
	const bool cond = (iform == XED_IFORM_DIV_GPR8) || (iform == XED_IFORM_DIV_MEMb);

	unsigned int value1;
	unsigned int value2;
	unsigned int result;
	unsigned int r;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			value2 = read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name));
			break;
		
		case XED_OPERAND_MEM0:
			value2 = cond ? readByte(genEA(op_name)) : readWord(genEA(op_name));
			break;
	}

	value1 = cond ? m_gregs[AX].l : m_gregs[AX].x;

	result = value1 / value2;
	r = value1 % value2;

	if (cond)
	{
		m_gregs[AX].l = result;
		m_gregs[AX].h = r;
	}
	else
	{
		m_gregs[AX].x = result;
		m_gregs[DX].x = r;
	}
}

//TODO: I am not sure about that !
void CPU::IDIV()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&m_decoded_inst);
	const bool cond = (iform == XED_IFORM_IDIV_GPR8) || (iform == XED_IFORM_IDIV_MEMb);

	int value1;
	int value2;
	int result;
	int r;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			value2 = (signed)read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name));
			break;
		
		case XED_OPERAND_MEM0:
			value2 = (signed)(cond ? readByte(genEA(op_name)) : readWord(genEA(op_name)));
			break;
	}

	value1 = cond ? m_gregs[AX].l : m_gregs[AX].x;

	result = value1 / value2;
	r = value1 % value2;

	if (cond)
	{
		m_gregs[AX].l = result;
		m_gregs[AX].h = r;
	}
	else
	{
		m_gregs[AX].x = result;
		m_gregs[DX].x = r;
	}
}