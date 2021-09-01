#include "arch.hpp"
#include "instructions.hpp"

static unsigned int twoOperandsInstruction (unsigned int (*instructionFunction)(unsigned int destOperand, unsigned int srcOperand))
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name0 = xed_operand_name(xed_inst_operand(inst, 0));
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst, 1));

	unsigned int destValue;
	unsigned int srcValue;

	switch (op_name1)
	{
		case XED_OPERAND_IMM0:
			srcValue = xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
			break;
		
		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			srcValue = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name1));
			break;

		// case XED_OPERAND_MEM0:
		// 	srcValue = cpu.eu.operandSizeWord ? cpu.biu.readWord(cpu.genEA()) : cpu.readByte(cpu.genEA());
		// 	break;

	}

	switch (op_name0)
	{
		case XED_OPERAND_REG0:
			destValue = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name0));
			destValue = instructionFunction(destValue,srcValue);
			cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name0),destValue);
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.genEA();
			destValue = cpu.eu.operandSizeWord ? cpu.biu.readWord(addr) : cpu.biu.readByte(addr);
			destValue = instructionFunction(destValue,srcValue);
			if (cpu.eu.operandSizeWord)
				cpu.biu.writeWord(addr,destValue);
			else
				cpu.biu.writeByte(addr,destValue);
			break;
		}
	}
	return destValue;
}

void NOT()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));

	if (op_name == XED_OPERAND_REG0)
		cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name), ~cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name)));
	else //MEM0 is the only possible other case
	{
		const unsigned int addr = cpu.genEA();
		if (cpu.eu.operandSizeWord)
			cpu.biu.writeWord(addr,~cpu.biu.readWord(addr));
		else
			cpu.biu.writeByte(addr,~cpu.biu.readByte(addr));
	}
}

void SHIFT	(void)
{
	const unsigned int result = twoOperandsInstruction([](const unsigned int destValue, const unsigned int shiftCount)
	{
		const unsigned int operandMSBMask = cpu.eu.operandSizeWord ? 0x8000 : 0x80;
		const bool instructionIsSAL_SHL = cpu.eu.instructionExtraData.directionIsLeft();
		const bool instructionIsSAR = cpu.eu.instructionExtraData.instructionIsArithmetic();
		
		unsigned int tempDest = instructionIsSAR ? (int)destValue : destValue;
		const unsigned int carryFlagValueMask = instructionIsSAL_SHL ? tempDest & operandMSBMask : tempDest & 0b1;

		for (int i = 0; i < shiftCount; ++i)
		{
			cpu.updateFlag(CPU::CARRY, tempDest & carryFlagValueMask);
			tempDest = instructionIsSAL_SHL ? tempDest * 2 : tempDest / 2; 
		}

		if (shiftCount == 1)
		{
			if (instructionIsSAL_SHL)
				cpu.updateFlag(CPU::OVER,(tempDest & operandMSBMask) ^ cpu.getFlagStatus(CPU::CARRY));
			else
				cpu.updateFlag(CPU::OVER,instructionIsSAR ? false : tempDest & operandMSBMask);
		}
		else if (shiftCount > 1)
			cpu.updateFlag(CPU::OVER,instructionIsSAR ? false : (destValue & operandMSBMask));

		return tempDest;
	});

	cpu.testSF(result);   cpu.testZF(result);   cpu.testPF(result);
}

void ROTATE(void)
{
	const unsigned int result = twoOperandsInstruction([](unsigned int value, unsigned int rotateCount)
	{
		unsigned int tempResult = value;
		if (cpu.eu.instructionExtraData.rotationWithCarry())
		{
			if (cpu.eu.instructionExtraData.directionIsLeft())
				tempResult |= cpu.getFlagStatus(CPU::CARRY) << (cpu.eu.operandSizeWord ? 16 : 8);
			else
				tempResult = (value << 1) | cpu.getFlagStatus(CPU::CARRY);
		}

		const unsigned int rotateValueInBound = rotateCount % (cpu.eu.operandSizeWord ? 16 : 8);
		const unsigned int rotateValueLeft = (cpu.eu.operandSizeWord ? 16 : 8) - rotateValueInBound;
		unsigned int rotateValueMask = 0;

		for (int i = 0; i < (cpu.eu.instructionExtraData.directionIsLeft() ? rotateValueLeft : rotateValueInBound); ++i)
			rotateValueMask |= 1 << i;

		const unsigned int resultUpperPartShift = (cpu.eu.instructionExtraData.directionIsLeft() ? rotateValueInBound : rotateValueLeft);
		const unsigned int resultUpperPart = (tempResult & rotateValueMask) << resultUpperPartShift;
		
		const unsigned int resultLowerPartShift = (cpu.eu.instructionExtraData.directionIsLeft() ? rotateValueLeft : rotateValueInBound);
		const unsigned int resultLowerPart = (tempResult & ~rotateValueMask) >> resultLowerPartShift;
		const unsigned int resultMSBMask = cpu.eu.operandSizeWord ? (1 << 16) : (1 << 8);
		unsigned int result = resultUpperPart | resultLowerPart;

		if (rotateCount != 0)
		{
			unsigned int newCarryFlagValue;
			if (cpu.eu.instructionExtraData.directionIsLeft())
				newCarryFlagValue = result & resultMSBMask;
			else
			{
				newCarryFlagValue = result & 1;
				if (cpu.eu.instructionExtraData.rotationWithCarry())
					result >>= 1;
			}
			cpu.updateStatusFlags(CPU::CARRY,newCarryFlagValue);
		}

		if (rotateCount == 1)
		{
			//Intel manual says that for ROR, OF = MSB(DEST) XOR MSB - 1(DEST)... WTF does it mean ?
			const unsigned int OFResultMask = cpu.eu.instructionExtraData.directionIsLeft() ? resultMSBMask : 1;
			cpu.updateFlag(CPU::OVER,(result & OFResultMask) ^ cpu.getFlagStatus(CPU::CARRY));
		}

		return result;
	});
}

// void SHL	(void);
// void SHR	(void);
// void SAR	(void);
// void ROL	(void){}
// void ROR	(void){}
// void RCL	(void){}
// void RCR	(void){}
void AND	(void){}
void TEST	(void){}
void OR		(void){}
void XOR	(void){}
