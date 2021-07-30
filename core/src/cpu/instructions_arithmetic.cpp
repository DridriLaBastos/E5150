#include "8086.hpp"
#include "instructions.hpp"

void ADD(CPU& _cpu)
{
	const unsigned int iform = xed_decoded_inst_get_iform_enum(&cpu.decodedInst);
	const bool cond = ((iform >= 33) && (iform <= 38)) || ((iform >= 44) && (iform <= 46));
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.decodedInst);
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst, 0));
	const xed_operand_enum_t op_name2 = xed_operand_name(xed_inst_operand(inst, 1));

	unsigned int value1;

	switch (op_name1)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&cpu.decodedInst, op_name1);
			value1 = cpu.readReg(reg);
			cpu.write_reg(reg, value1);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value1 = cond ? cpu.readByte(addr) : cpu.readWord(addr);

			if (cond)
				cpu.writeByte(addr, value1);
			else
				cpu.writeWord(addr, value1);
		} break;
	}

	switch (op_name2)
	{
		case XED_OPERAND_IMM0:
			value1 += xed_decoded_inst_get_unsigned_immediate(&cpu.decodedInst);
			break;
		
		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			value1 += cpu.readReg(xed_decoded_inst_get_reg(&cpu.decodedInst, op_name2));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value1 += cond ? cpu.readByte(addr) : cpu.readWord(addr);
		} break;
	}

	cpu.updateStatusFlags(value1, cond ? 1 : 2);
}

void INC(CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.decodedInst), 0));

	const unsigned int iform = xed_decoded_inst_get_iform_enum(&cpu.decodedInst);
	const bool cond = (iform == XED_IFORM_INC_GPR8) || (iform == XED_IFORM_INC_MEMb) || (iform == XED_IFORM_INC_LOCK_MEMb);

	unsigned int value1;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&cpu.decodedInst, op_name);
			value1 = cpu.readReg(reg) + 1;
			cpu.write_reg(reg, value1);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value1 = (cond ? cpu.readByte(addr) : cpu.readWord(addr)) + 1;

			if (cond)
				cpu.writeByte(addr, value1);
			else
				cpu.writeWord(addr, value1);
		} break;
	}

	cpu.updateStatusFlags(value1, cond ? 1 : 2);
}

void SUB(CPU& _cpu)
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.decodedInst);
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst, 0));
	const xed_operand_enum_t op_name2 = xed_operand_name(xed_inst_operand(inst, 1));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&cpu.decodedInst);
	const bool cond = 	((iform >= XED_IFORM_SUB_AL_IMMb) && (iform >= XED_IFORM_SUB_GPR8_MEMb)) ||
					  	((iform >= XED_IFORM_SUB_MEMb_GPR8) && (iform <= XED_IFORM_SUB_MEMb_IMMb_82r5));

	unsigned int value1;

	switch (op_name1)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&cpu.decodedInst, op_name1);

			value1 = cpu.readReg(reg);
			cpu.write_reg(reg, value1);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value1 = cond ? cpu.readByte(addr) : cpu.readWord(addr);

			if (cond) cpu.writeByte(addr, value1); else cpu.writeWord(addr, value1);
		} break;
	}

	switch (op_name2)
	{
		case XED_OPERAND_IMM0:
			value1 -= xed_decoded_inst_get_unsigned_immediate(&cpu.decodedInst);
			break;
		
		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			value1 -= cpu.readReg(xed_decoded_inst_get_reg(&cpu.decodedInst, op_name2));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value1 -= cond ? cpu.readByte(addr) : cpu.readWord(addr);
		} break;
	}

	cpu.updateStatusFlags(value1, cond ? 1 : 2);
}

void DEC(CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.decodedInst), 0));
	const xed_iform_enum_t   iform   = xed_decoded_inst_get_iform_enum(&cpu.decodedInst);
	const bool cond = (iform == XED_IFORM_DEC_GPR8) || (iform == XED_IFORM_DEC_MEMb) || (XED_IFORM_DEC_LOCK_MEMb);

	unsigned int value1;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&cpu.decodedInst, op_name);

			value1 = cpu.readReg(reg) - 1;
			cpu.write_reg(reg, value1);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value1 = (cond ? cpu.readByte(addr) : cpu.readWord(addr)) - 1;

			if (cond)
				cpu.writeByte(addr, value1);
			else
				cpu.writeWord(addr, value1);
		} break;
	}

	cpu.updateStatusFlags(value1, cond ? 1 : 2);
}

void NEG(CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.decodedInst), 0));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&cpu.decodedInst);
	const bool cond = (iform == XED_IFORM_NEG_GPR8) || (iform == XED_IFORM_NEG_MEMb) || (iform == XED_IFORM_NEG_LOCK_MEMb);

	unsigned int value1;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&cpu.decodedInst, op_name);

			value1 = cpu.readReg(reg);
			value1 *= -1;
			cpu.write_reg(reg, value1);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();

			value1 = cond ? cpu.readByte(addr) : cpu.readWord(addr);
			value1 *= -1;

			if (cond)
				cpu.writeByte(addr, value1);
			else
				cpu.writeWord(addr, value1);
		} break;
	}

	if (value1 == 0)
		cpu.clearFlags(CPU::CARRY);
	else
		cpu.setFlags(CPU::CARRY);
	
	cpu.updateStatusFlags(value1, cond ? 1: 2);
}

void CMP(CPU& _cpu)
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.decodedInst);
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst, 0));
	const xed_operand_enum_t op_name2 = xed_operand_name(xed_inst_operand(inst, 1));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&cpu.decodedInst);
	const bool cond = 	((iform >= XED_IFORM_CMP_AL_IMMb) && (iform <= XED_IFORM_CMP_GPR8_MEMb)) ||
						((iform >= XED_IFORM_CMP_MEMb_GPR8) && (iform <= XED_IFORM_CMP_MEMb_IMMb_82r7));

	unsigned int value1 = 0;

	switch (op_name1)
	{
		case XED_OPERAND_REG0:
			value1 += cpu.readReg(xed_decoded_inst_get_reg(&cpu.decodedInst, op_name1));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value1 += cond ? cpu.readByte(addr) : cpu.readWord(addr);
		} break;
	}

	switch (op_name2)
	{
		case XED_OPERAND_IMM0:
			value1 -= xed_decoded_inst_get_unsigned_immediate(&cpu.decodedInst);
			break;
			
		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			value1 -= cpu.readReg(xed_decoded_inst_get_reg(&cpu.decodedInst, op_name2));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value1 -= cond ? cpu.readByte(addr) : cpu.readWord(addr);
		} break;
	}

	cpu.updateStatusFlags(value1, cond ? 1 : 2);
}

void MUL(CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.decodedInst), 0));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&cpu.decodedInst);
	const bool cond = (iform == XED_IFORM_MUL_GPR8) || (iform == XED_IFORM_MUL_MEMb);

	unsigned int value1 = 1;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			value1 *= cpu.readReg(xed_decoded_inst_get_reg(&cpu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_MEM0:
			value1 *= cond ? cpu.readByte(cpu.genEA()) : cpu.readWord(cpu.genEA());
			break;
	}

	value1 *= cond ? cpu.regs[CPU::AX].l : cpu.regs[CPU::AX].x;

	cpu.regs[CPU::AX].x = value1;

	if (!cond)
		cpu.regs[CPU::DX].x = value1 >> 16;
	
	if (cond)
	{
		if (cpu.regs[CPU::AX].h == 0)
			cpu.clearFlags(CPU::CARRY|CPU::OVER);
		else
			cpu.setFlags(CPU::CARRY|CPU::OVER);
	}
	else
	{
		if (cpu.regs[CPU::DX].x == 0)
			cpu.clearFlags(CPU::CARRY|CPU::OVER);
		else
			cpu.setFlags(CPU::CARRY|CPU::OVER);
	}
}

void IMUL(CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.decodedInst), 0));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&cpu.decodedInst);
	const bool cond = (iform == XED_IFORM_IMUL_GPR8) || (iform == XED_IFORM_IMUL_MEMb);

	int value1 = 1;

	switch(op_name)
	{
		case XED_OPERAND_REG0:
			value1 *= (signed)cpu.readReg(xed_decoded_inst_get_reg(&cpu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_MEM0:
			value1 *= (signed)(cond ? cpu.readByte(cpu.genEA()) : cpu.readWord(cpu.genEA()));
			break;
	}

	value1 *= (signed)(cond ? cpu.regs[CPU::AX].l : cpu.regs[CPU::AX].x); 

	cpu.regs[CPU::AX].x = value1;

	if (!cond)
		cpu.regs[CPU::DX].x = value1 >> 16;
	
	if (cond)
	{
		if (cpu.regs[CPU::AX].h == 0)
			cpu.clearFlags(CPU::CARRY|CPU::OVER);
		else
			cpu.setFlags(CPU::CARRY|CPU::OVER);
	}
	else
	{
		if (cpu.regs[CPU::DX].x == 0)
			cpu.clearFlags(CPU::CARRY|CPU::OVER);
		else
			cpu.setFlags(CPU::CARRY|CPU::OVER);
	}
}

//TODO: what happens when dividing by zero ? Restart ?
void DIV(CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.decodedInst), 0));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&cpu.decodedInst);
	const bool cond = (iform == XED_IFORM_DIV_GPR8) || (iform == XED_IFORM_DIV_MEMb);

	unsigned int value1;
	unsigned int value2;
	unsigned int result;
	unsigned int r;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			value2 = cpu.readReg(xed_decoded_inst_get_reg(&cpu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_MEM0:
			value2 = cond ? cpu.readByte(cpu.genEA()) : cpu.readWord(cpu.genEA());
			break;
	}

	value1 = cond ? cpu.regs[CPU::AX].l : cpu.regs[CPU::AX].x;

	result = value1 / value2;
	r = value1 % value2;

	if (cond)
	{
		cpu.regs[CPU::AX].l = result;
		cpu.regs[CPU::AX].h = r;
	}
	else
	{
		cpu.regs[CPU::AX].x = result;
		cpu.regs[CPU::DX].x = r;
	}
}

//TODO: I am not sure about that !
void IDIV(CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.decodedInst), 0));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&cpu.decodedInst);
	const bool cond = (iform == XED_IFORM_IDIV_GPR8) || (iform == XED_IFORM_IDIV_MEMb);

	int value1;
	int value2;
	int result;
	int r;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			value2 = (signed)cpu.readReg(xed_decoded_inst_get_reg(&cpu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_MEM0:
			value2 = (signed)(cond ? cpu.readByte(cpu.genEA()) : cpu.readWord(cpu.genEA()));
			break;
	}

	value1 = cond ? cpu.regs[CPU::AX].l : cpu.regs[CPU::AX].x;

	result = value1 / value2;
	r = value1 % value2;

	if (cond)
	{
		cpu.regs[CPU::AX].l = result;
		cpu.regs[CPU::AX].h = r;
	}
	else
	{
		cpu.regs[CPU::AX].x = result;
		cpu.regs[CPU::DX].x = r;
	}
}