#include "arch.hpp"
#include "instructions.hpp"

//TODO: Investigate how to do it with variadic template
template <bool OVERWRITE_DEST = true>
static unsigned int twoOperandsInstruction(
	unsigned int (*instructionAction)(const unsigned int destOperand, const unsigned int srcOperand, const bool x), const bool extra)
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name0 = xed_operand_name(xed_inst_operand(inst,0));
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst,1));

	unsigned int srcOperand;
	unsigned int destOperand;
	unsigned int result;

	switch (op_name1)
	{
		case XED_OPERAND_IMM0:
			srcOperand = xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
			break;

		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			srcOperand = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name1));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			srcOperand = cpu.eu.operandSizeWord ? cpu.biu.readWord(addr) : cpu.biu.readByte(addr);
		} break;
	}

	switch (op_name0)
	{
		case XED_OPERAND_REG0:
			destOperand = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name0));
			result = instructionAction(destOperand,srcOperand,extra);
			if constexpr (OVERWRITE_DEST)
				cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name0),result);
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			destOperand = cpu.eu.operandSizeWord ? cpu.biu.readWord(addr) : cpu.biu.readByte(addr);
			if constexpr (OVERWRITE_DEST)
				result = instructionAction(destOperand,srcOperand,extra);
		} break;
	}
	return result;
}

static unsigned int oneOperandInstruction(
	unsigned int (*instructionAction)(const unsigned int destOperand, const bool x), const bool extra)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	unsigned int result;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg = xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name);
			result = instructionAction(cpu.readReg(reg),extra);
			cpu.write_reg(reg, result);
		} break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();

			if (cpu.eu.operandSizeWord)
			{
				result = instructionAction(cpu.biu.readWord(addr),extra);
				cpu.biu.writeWord(result,addr);
			}
			else
			{
				result = instructionAction(cpu.biu.readByte(addr),extra);
				cpu.biu.writeByte(result,addr);
			}
		} break;
	}

	return result;
}

void ADD(const bool withCarry)
{
	const unsigned int result = twoOperandsInstruction([](const unsigned int destOperand, const unsigned int srcOperand, const bool withCarry) { return destOperand + srcOperand + withCarry;},withCarry);
	cpu.updateStatusFlags(result,cpu.eu.operandSizeWord);
}

void INC()
{
	const unsigned int result = oneOperandInstruction([](const unsigned int operand,bool){ return operand+1; },false);
	cpu.updateStatusFlags(result, cpu.eu.operandSizeWord);
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
	const unsigned int result = twoOperandsInstruction([](const unsigned int destOperand, const unsigned int srcOperand, const bool extra) { return destOperand - (srcOperand + extra); },withCarry);
	cpu.updateStatusFlags(result,cpu.eu.operandSizeWord);
}

void DEC()
{
	const unsigned int result = oneOperandInstruction([](const unsigned int destOperand, const bool) { return destOperand - 1; },false);
	cpu.updateStatusFlags(result,cpu.eu.operandSizeWord);
}

void NEG()
{
	const unsigned int result = oneOperandInstruction([](const unsigned int destOperand, const bool) { return - destOperand; },false);
	cpu.updateStatusFlags(result, cpu.eu.operandSizeWord);
}

void CMP()
{
	const unsigned int result = twoOperandsInstruction<false>([](const unsigned int destOperand, const unsigned int srcOperand, const bool) { return destOperand - srcOperand; },false);
	cpu.updateStatusFlags(result,cpu.eu.operandSizeWord);
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
	unsigned int result = oneOperandInstruction([](const unsigned int destOperand, const bool isSigned) { return  (isSigned) ? (int)destOperand : destOperand;},isSigned);
	
	unsigned int half = 0; //upper half in unsigned mode, lower half in signed mode

	if (cpu.eu.operandSizeWord)
	{
		result *= cpu.ax;
		cpu.ax = result;
		cpu.dx = result >> 16;
		half = isSigned ? (int)cpu.ax : cpu.dx;
	}
	else
	{
		result *= cpu.al;
		cpu.ax = result;
		half = isSigned ? (int)cpu.al : cpu.ah;
	}

	const bool clearFlagsCondition = isSigned ? (half == result) : (half == 0);

	if (clearFlagsCondition) { cpu.clearFlags(CPU::OVER | CPU::CARRY); }
	else { cpu.setFlags(CPU::OVER | CPU::CARRY); }
}

//TODO: what happens when dividing by zero ? Restart ?
//TODO: continue working on the signed division
void DIV(const bool isSigned)
{
	const unsigned int src = oneOperandInstruction([](const unsigned int destOperand, const bool isSigned) { return  (isSigned) ? (int)destOperand : destOperand;},isSigned);
	unsigned int result;

	if (cpu.eu.operandSizeWord)
	{
		const unsigned int dx_ax = (cpu.dx << 16) | cpu.ax;
		result = dx_ax / src;
		//if (result > 0xFFFF) #DE (* Divide error *)
		//if (result > 0x7FFF) or (result < 0x80000) #DE (* Divide error *) for signed
		cpu.ax = result;
		cpu.dx = dx_ax % src;
	}
	else
	{
		result = cpu.ax / src;
		//if (result > 0xFF) #DE (* Divide error *)
		//if (result > 0x7F) ro (result < 0x80) #DE (* Divide error *) for signed
		cpu.al = result;
		cpu.ah = cpu.ax % src;
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