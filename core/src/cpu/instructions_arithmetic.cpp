#include "arch.hpp"
#include "instructions.hpp"

void ADD(const bool withCarry)
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name0 = xed_operand_name(xed_inst_operand(inst,0));
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst,1));
	const bool operandSizeByte = !(cpu.biu.instructionBufferQueue[0] & 1);//The last bit of the instruction is w (byte if w = 0 and word if 1 = 1)

	unsigned int value = withCarry ? cpu.getFlagStatus(CPU::CARRY) : 0;

	switch (op_name1)
	{
		case XED_OPERAND_IMM0:
			value += xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
			break;

		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			value += cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name1));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value += operandSizeByte ? cpu.biu.readByte(addr) : cpu.biu.readWord(addr);
		} break;
	}

	switch (op_name0)
	{
		case XED_OPERAND_REG0:
			value += cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name0));
			cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name0),value);
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value += operandSizeByte ? cpu.biu.readByte(addr) : cpu.biu.readWord(addr);
		} break;
	}

	cpu.updateStatusFlags(value, operandSizeByte);
}

void INC()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	const bool operandSizeByte = !(cpu.biu.instructionBufferQueue[0] & 0xF);

	unsigned int value;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name);
			value = cpu.readReg(reg) + 1;
			cpu.write_reg(reg, value);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value = (operandSizeByte ? cpu.biu.readByte(addr) : cpu.biu.readWord(addr)) + 1;
		} break;
	}

	cpu.updateStatusFlags(value, operandSizeByte);
}

void AAA()
{
	if (((cpu.al & 0xF) > 9) || cpu.getFlagStatus(CPU::A_CARRY))
	{
		cpu.ax += 0x106;
		cpu.setFlags(CPU::A_CARRY | CPU::CARRY);
	}
	else
		cpu.clearFlags(CPU::A_CARRY | CPU::CARRY);
	
	cpu.al &= 0xF;
}

void DAA()
{
	const unsigned int oldAL = cpu.al;
	const bool oldCF = cpu.getFlagStatus(CPU::CARRY);
	cpu.clearFlags(CPU::CARRY);

	if (((cpu.al & 0xF) > 9) || cpu.getFlagStatus(CPU::A_CARRY))
	{
		cpu.al += 6;
		cpu.updateFlag(CPU::CARRY,oldCF || ((oldAL+6) & 0xFF00));
		cpu.setFlags(CPU::A_CARRY);
	}
	else
		cpu.clearFlags(CPU::A_CARRY);
	
	if ((oldAL > 0x99) || oldCF)
	{
		cpu.al += 0x60;
		cpu.setFlags(CPU::CARRY);
	}
	else
		cpu.clearFlags(CPU::CARRY);
}

void SUB(const bool withCarry)
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name0 = xed_operand_name(xed_inst_operand(inst, 0));
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst, 1));
	const bool operandSizeByte = !(cpu.biu.instructionBufferQueue[0] & 0b1);

	unsigned int value = withCarry ? cpu.getFlagStatus(CPU::CARRY) : 0;

	switch (op_name1)
	{
		case XED_OPERAND_IMM0:
			value += xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
			break;

		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			value += cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name1));
			break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value += operandSizeByte ? cpu.biu.readByte(addr) : cpu.biu.readWord(addr);
		} break;
	}

	switch (op_name0)
	{
		case XED_OPERAND_REG0:
			value = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name0)) - value;
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value = (operandSizeByte ? cpu.biu.readByte(addr) : cpu.biu.readWord(addr)) - value;
		} break;
	}

	cpu.updateStatusFlags(value, operandSizeByte);
}

void DEC()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	const bool operandSizeByte = !(cpu.biu.instructionBufferQueue[0] & 0xF);

	unsigned int value;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name);
			value = cpu.readReg(reg) - 1;
			cpu.write_reg(reg, value);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value = (operandSizeByte ? cpu.biu.readByte(addr) : cpu.biu.readWord(addr)) - 1;
		} break;
	}

	cpu.updateStatusFlags(value, operandSizeByte);
}

void NEG()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&cpu.eu.decodedInst);
	const bool operandSizeByte = !(cpu.biu.instructionBufferQueue[0] & 0b1);

	unsigned int value;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name);
			value = -cpu.readReg(reg);
			cpu.write_reg(reg, value);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value = -(operandSizeByte ? cpu.biu.readByte(addr) : cpu.biu.readWord(addr));
		} break;
	}

	cpu.updateStatusFlags(value, operandSizeByte);
}

void CMP()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name0 = xed_operand_name(xed_inst_operand(inst, 0));
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst, 1));
	const bool operandSizeByte = !(cpu.biu.instructionBufferQueue[0] & 0b1);

	unsigned int value;

	switch (op_name0)
	{
		case XED_OPERAND_REG0:
			value = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name1));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value = operandSizeByte ? cpu.biu.readByte(addr) : cpu.biu.readWord(addr);
		} break;
	}

	switch (op_name1)
	{
		case XED_OPERAND_IMM0:
			value -= xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
			break;
			
		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			value -= cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name1));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			value -= operandSizeByte ? cpu.biu.readByte(addr) : cpu.biu.readWord(addr);
		} break;
	}

	cpu.updateStatusFlags(value,operandSizeByte);
}

void AAS()
{
	if (((cpu.al & 0xF) > 9) || cpu.getFlagStatus(CPU::A_CARRY))
	{
		cpu.ax -= 6;
		cpu.al -= 1;
		cpu.setFlags(CPU::A_CARRY | CPU::CARRY);
	}
	else
		cpu.clearFlags(CPU::A_CARRY | CPU::CARRY);

	cpu.al &= 0xF;
}

void DAS()
{
	const unsigned int oldAL = cpu.al;
	const bool oldCF = cpu.getFlagStatus(CPU::CARRY);
	cpu.clearFlags(CPU::CARRY);

	if (((cpu.al & 0xF) > 9) || cpu.getFlagStatus(CPU::A_CARRY))
	{
		cpu.al -= 6;
		cpu.setFlags(CPU::A_CARRY);
		cpu.updateFlag(CPU::CARRY,oldCF || ((oldAL - 6) & 0xFF00));
	}
	else
		cpu.clearFlags(CPU::A_CARRY);
	
	if ((oldAL > 0x99) || oldCF)
	{
		cpu.al -= 0x60;
		cpu.setFlags(CPU::CARRY);
	}
}

void MUL(const bool isSigned)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	const bool operandSizeByte = !(cpu.biu.instructionBufferQueue[0] & 0b1);

	unsigned int value;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			value = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_MEM0:
			value = operandSizeByte ? cpu.biu.readByte(cpu.genEA()) : cpu.biu.readWord(cpu.genEA());
			break;
	}
	
	unsigned int half = 0; //upper half in unsigned mode, lower half in signed mode

	if (operandSizeByte)
	{
		value *= cpu.al;
		cpu.ax = value;
		half = isSigned ? (int)cpu.al : cpu.ah;
	}
	else
	{
		value *= cpu.ax;
		cpu.ax = value;
		cpu.dx = value >> 16;
		half = isSigned ? (int)cpu.ax : cpu.dx;
	}

	const bool clearFlagsCondition = isSigned ? (half == value) : (half == 0);

	if (clearFlagsCondition) { cpu.clearFlags(CPU::OVER | CPU::CARRY); }
	else { cpu.setFlags(CPU::OVER | CPU::CARRY); }
}

//TODO: what happens when dividing by zero ? Restart ?
//TODO: continue working on the signed division
void DIV(const bool isSigned)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	const bool operandSizeByte = !(cpu.biu.instructionBufferQueue[0] & 0b1);

	unsigned int src;
	unsigned int result;
	unsigned int r;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			src = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_MEM0:
			src = operandSizeByte ? cpu.biu.readByte(cpu.genEA()) : cpu.biu.readWord(cpu.genEA());
			break;
	}

	if (operandSizeByte)
	{
		result = cpu.ax / src;
		//if (result > 0xFF) #DE (* Divide error *)
		//if (result > 0x7F) ro (result < 0x80) #DE (* Divide error *) for signed
		cpu.al = result;
		cpu.ah = cpu.ax % src;
	}
	else
	{
		const unsigned int dx_ax = (cpu.dx << 16) | cpu.ax;
		result = dx_ax / src;
		//if (result > 0xFFFF) #DE (* Divide error *)
		//if (result > 0x7FFF) or (result < 0x80000) #DE (* Divide error *) for signed
		cpu.ax = result;
		cpu.dx = dx_ax % src;
	}
}

void AAD()
{
	cpu.ax = (cpu.al + (cpu.ah * 0xA));
	cpu.ah = 0;
	cpu.testSF(cpu.al);   cpu.testZF(cpu.al);   cpu.testPF(cpu.al);
}

void CBW()
{ cpu.ax = (int16_t)cpu.al; }

void CWD()
{ cpu.dx = ((int32_t)cpu.ax >> 16); }