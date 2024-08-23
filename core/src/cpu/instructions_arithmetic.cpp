#include "core/arch.hpp"
#include "core/instructions.hpp"

//TODO: Invdestigate how to do it with variaregs.dic template
template <bool OVERWRITE_DEST = true>
static unsigned int Arithmetic_twoOperandsInstruction(
	unsigned int (*instructionAction)(const unsigned int destOperand, const unsigned int srcOperand))
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu->decodedInst);
	const xed_operand_enum_t op_name0 = xed_operand_name(xed_inst_operand(inst,0));
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst,1));

	unsigned int srcOperand;
	unsigned int destOperand;
	unsigned int result;

	switch (op_name1)
	{
		case XED_OPERAND_IMM0:
			srcOperand = xed_decoded_inst_get_unsigned_immediate(&cpu->decodedInst);
			break;

		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			srcOperand = cpu.readReg(xed_decoded_inst_get_reg(&cpu->decodedInst,op_name1));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.eu.EAddress;
			srcOperand = cpu.eu.operandSizeWord ? cpu.biu.readWord(addr) : cpu.biu.readByte(addr);
		} break;
	}

	switch (op_name0)
	{
		case XED_OPERAND_REG0:
			destOperand = cpu.readReg(xed_decoded_inst_get_reg(&cpu->decodedInst,op_name0));
			result = instructionAction(destOperand,srcOperand);
			if constexpr (OVERWRITE_DEST)
				cpu.write_reg(xed_decoded_inst_get_reg(&cpu->decodedInst,op_name0),result);
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.eu.EAddress;
			destOperand = cpu.eu.operandSizeWord ? cpu.biu.readWord(addr) : cpu.biu.readByte(addr);
			result = instructionAction(destOperand,srcOperand);
			if constexpr (OVERWRITE_DEST)
			{
				if (cpu.eu.operandSizeWord)
					cpu.biu.writeWord(addr, result);
				else
					cpu.biu.writeByte(addr, result);
			}
		} break;
	}
	return result;
}

static unsigned int oneOperandInstruction(
	unsigned int (*instructionAction)(const unsigned int destOperand))
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu->decodedInst), 0));
	unsigned int result;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&cpu->decodedInst, op_name);
			result = instructionAction(cpu.readReg(reg));
			cpu.write_reg(reg, result);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.eu.EAddress;

			if (cpu.eu.operandSizeWord)
			{
				result = instructionAction(cpu.biu.readWord(addr));
				cpu.biu.writeWord(result,addr);
			}
			else
			{
				result = instructionAction(cpu.biu.readByte(addr));
				cpu.biu.writeByte(result,addr);
			}
		} break;
	}

	return result;
}

void ADD()
{
	const unsigned int result = Arithmetic_twoOperandsInstruction([](const unsigned int destOperand, const unsigned int srcOperand) { return destOperand + srcOperand + cpu.eu.instructionExtraData.withCarry;});
	cpu.updateStatusFlags(result,cpu.eu.operandSizeWord);
}

void INC()
{
	const unsigned int result = oneOperandInstruction([](const unsigned int operand){ return operand+1; });
	cpu.updateStatusFlags(result, cpu.eu.operandSizeWord);
}

void AAA()
{
	if (((cpu.regs.al & 0xF) > 9) || cpu.getFlagStatus(CPU::A_CARRY))
	{
		cpu.regs.ax += 0x106;
		cpu.setFlags(CPU::A_CARRY | CPU::CARRY);
	}
	else
		cpu.clearFlags(CPU::A_CARRY | CPU::CARRY);
	
	cpu.regs.al &= 0xF;
}

void DAA()
{
	const unsigned int oldAL = cpu.regs.al;
	const bool oldCF = cpu.getFlagStatus(CPU::CARRY);
	cpu.clearFlags(CPU::CARRY);

	if (((cpu.regs.al & 0xF) > 9) || cpu.getFlagStatus(CPU::A_CARRY))
	{
		cpu.regs.al += 6;
		cpu.updateFlag(CPU::CARRY,oldCF || ((oldAL+6) & 0xFF00));
		cpu.setFlags(CPU::A_CARRY);
	}
	else
		cpu.clearFlags(CPU::A_CARRY);
	
	if ((oldAL > 0x99) || oldCF)
	{
		cpu.regs.al += 0x60;
		cpu.setFlags(CPU::CARRY);
	}
	else
		cpu.clearFlags(CPU::CARRY);
}

void SUB(void)
{
	const unsigned int result = Arithmetic_twoOperandsInstruction([](const unsigned int destOperand, const unsigned int srcOperand) { return destOperand - (srcOperand + cpu.eu.instructionExtraData.withCarry); });
	cpu.updateStatusFlags(result,cpu.eu.operandSizeWord);
}

void DEC()
{
	const unsigned int result = oneOperandInstruction([](const unsigned int destOperand) { return destOperand - 1; });
	cpu.updateStatusFlags(result,cpu.eu.operandSizeWord);
}

void NEG()
{
	const unsigned int result = oneOperandInstruction([](const unsigned int destOperand) { return - destOperand; });
	cpu.updateStatusFlags(result, cpu.eu.operandSizeWord);
}

void CMP()
{
	const unsigned int result = Arithmetic_twoOperandsInstruction<false>([](const unsigned int destOperand, const unsigned int srcOperand) { return destOperand - srcOperand; });
	cpu.updateStatusFlags(result,cpu.eu.operandSizeWord);
}

void AAS()
{
	if (((cpu.regs.al & 0xF) > 9) || cpu.getFlagStatus(CPU::A_CARRY))
	{
		cpu.regs.ax -= 6;
		cpu.regs.al -= 1;
		cpu.setFlags(CPU::A_CARRY | CPU::CARRY);
	}
	else
		cpu.clearFlags(CPU::A_CARRY | CPU::CARRY);

	cpu.regs.al &= 0xF;
}

void DAS()
{
	const unsigned int oldAL = cpu.regs.al;
	const bool oldCF = cpu.getFlagStatus(CPU::CARRY);
	cpu.clearFlags(CPU::CARRY);

	if (((cpu.regs.al & 0xF) > 9) || cpu.getFlagStatus(CPU::A_CARRY))
	{
		cpu.regs.al -= 6;
		cpu.setFlags(CPU::A_CARRY);
		cpu.updateFlag(CPU::CARRY,oldCF || ((oldAL - 6) & 0xFF00));
	}
	else
		cpu.clearFlags(CPU::A_CARRY);
	
	if ((oldAL > 0x99) || oldCF)
	{
		cpu.regs.al -= 0x60;
		cpu.setFlags(CPU::CARRY);
	}
}

void MUL()
{
	unsigned int result = oneOperandInstruction([](const unsigned int destOperand) { return  (cpu.eu.instructionExtraData.isSigned) ? (int)destOperand : destOperand;});
	
	unsigned int half = 0; //upper half in unsigned mode, lower half in regs.signed mode

	if (cpu.eu.operandSizeWord)
	{
		result *= cpu.regs.ax;
		cpu.regs.ax = result;
		cpu.regs.dx = result >> 16;
		half = cpu.eu.instructionExtraData.isSigned ? (int)cpu.regs.ax : cpu.regs.dx;
	}
	else
	{
		result *= cpu.regs.al;
		cpu.regs.ax = result;
		half = cpu.eu.instructionExtraData.isSigned ? (int)cpu.regs.al : cpu.regs.ah;
	}

	const bool clearFlagsCondition = cpu.eu.instructionExtraData.isSigned ? (half == result) : (half == 0);

	if (clearFlagsCondition) { cpu.clearFlags(CPU::OVER | CPU::CARRY); }
	else { cpu.setFlags(CPU::OVER | CPU::CARRY); }
}

//TODO: what happens when regs.diviregs.ding by zero ? Rdestart ?
//TODO: continue working on the regs.signed regs.diviregs.sion
//TODO: Divide interrupt
void DIV()
{
	const unsigned int src = oneOperandInstruction([](const unsigned int destOperand) { return  (cpu.eu.instructionExtraData.isSigned) ? (int)destOperand : destOperand;});
	unsigned int result;

	if (cpu.eu.operandSizeWord)
	{
		const unsigned int dx_ax = (cpu.regs.dx << 16) | cpu.regs.ax;
		result = dx_ax / src;
		//if (result > 0xFFFF) #DE (* Divide error *)
		//if (result > 0x7FFF) or (result < 0x80000) #DE (* Divide error *) for regs.signed
		cpu.regs.ax = result;
		cpu.regs.dx = dx_ax % src;
	}
	else
	{
		result = cpu.regs.ax / src;
		//if (result > 0xFF) #DE (* Divide error *)
		//if (result > 0x7F) ro (result < 0x80) #DE (* Divide error *) for regs.signed
		cpu.regs.al = result;
		cpu.regs.ah = cpu.regs.ax % src;
	}
}

void AAD()
{
	cpu.regs.ax = (cpu.regs.al + (cpu.regs.ah * 0xA));
	cpu.regs.ah = 0;
	cpu.testSF(cpu.regs.al);   cpu.testZF(cpu.regs.al);   cpu.testPF(cpu.regs.al);
}

void CBW()
{ cpu.regs.ax = (int16_t)cpu.regs.al; }

void CWD()
{ cpu.regs.dx = ((int32_t)cpu.regs.ax >> 16); }