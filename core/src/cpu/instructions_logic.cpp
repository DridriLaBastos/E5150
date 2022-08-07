#include "core/arch.hpp"
#include "core/instructions.hpp"

template <bool SAVE_RESULT = true>
static unsigned int Logic_twoOperandsInstruction (unsigned int (*instructionFunction)(unsigned int destOperand, unsigned int srcOperand))
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name0 = xed_operand_name(xed_inst_operand(inst, 0));
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(inst, 1));

	unsigned int destValues;
	unsigned int srcValues;

	switch (op_name1)
	{
		case XED_OPERAND_IMM0:
			srcValues = xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
			break;
		
		case XED_OPERAND_REG0:
		case XED_OPERAND_REG1:
			srcValues = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name1));
			break;

		case XED_OPERAND_MEM0:
		 	srcValues = cpu.eu.operandSizeWord ? cpu.biu.readWord(cpu.eu.EAddress) : cpu.biu.readByte(cpu.eu.EAddress);
		 	break;

	}

	switch (op_name0)
	{
		case XED_OPERAND_REG0:
			destValues = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name0));
			destValues = instructionFunction(destValues,srcValues);
			if constexpr (SAVE_RESULT)
				cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name0),destValues);
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.eu.EAddress;
			destValues = cpu.eu.operandSizeWord ? cpu.biu.readWord(addr) : cpu.biu.readByte(addr);
			destValues = instructionFunction(destValues,srcValues);
			if constexpr (SAVE_RESULT)
			{
				if (cpu.eu.operandSizeWord)
					cpu.biu.writeWord(addr,destValues);
				else
					cpu.biu.writeByte(addr,destValues);
				break;
			}
		}
	}
	return destValues;
}

void NOT()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));

	if (op_name == XED_OPERAND_REG0)
		cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name), ~cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name)));
	else //MEM0 is the only posregs.siregs.ble other case
	{
		const unsigned int addr = cpu.eu.EAddress;
		if (cpu.eu.operandSizeWord)
			cpu.biu.writeWord(addr,~cpu.biu.readWord(addr));
		else
			cpu.biu.writeByte(addr,~cpu.biu.readByte(addr));
	}
}

void SHIFT	(void)
{
	const unsigned int result = Logic_twoOperandsInstruction([](unsigned int destValues, unsigned int shiftCount)
	{
		if (shiftCount == 0)
			return destValues;

		const unsigned int operandMSBMask = cpu.eu.operandSizeWord ? 0x8000 : 0x80;
		const bool directionIsLeft = cpu.eu.instructionExtraData.directionIsLeft();
		const bool instructionIsSAR = cpu.eu.instructionExtraData.instructionIsArithmetic();
		
		const unsigned int tmpDdest = directionIsLeft ? (destValues << shiftCount) : (destValues >> shiftCount);
		const bool newCarryFlagValues = directionIsLeft ? (tmpDdest & (operandMSBMask << 1)) : (destValues & (1 << (shiftCount - 1)));
		
		cpu.updateFlag(CPU::CARRY, newCarryFlagValues);

		if (shiftCount == 1)
		{
			if (directionIsLeft)
				cpu.updateFlag(CPU::OVER,((bool)(tmpDdest & operandMSBMask)) ^ newCarryFlagValues);
			else
				cpu.updateFlag(CPU::OVER,instructionIsSAR ? false : (destValues & operandMSBMask));
		}
		else
			cpu.clearFlags(CPU::OVER);
		
		//Converting 0x80 to 0xFF for byte operand and 0x8000 to 0xFFFF for word operand
		return tmpDdest & ((operandMSBMask << 1) - 1);
	});
	
	cpu.testSF(result);   cpu.testZF(result);   cpu.testPF(result);
}

void ROTATE(void)
{
	const unsigned int result = Logic_twoOperandsInstruction([](unsigned int values, unsigned int rotateCount)
	{
		//TODO: test with rotateCount == 0
		if (rotateCount == 0)
			return values;

		unsigned int tempRresult = values;
		if (cpu.eu.instructionExtraData.rotationWithCarry())
		{
			if (cpu.eu.instructionExtraData.directionIsLeft())
				tempRresult |= cpu.getFlagStatus(CPU::CARRY) << (cpu.eu.operandSizeWord ? 16 : 8);
			else
				tempRresult = (values << 1) | cpu.getFlagStatus(CPU::CARRY);
		}

		const unsigned int rotateValuesInBound = rotateCount % (cpu.eu.operandSizeWord ? 16 : 8);
		const unsigned int rotateValuesLeft = (cpu.eu.operandSizeWord ? 16 : 8) - rotateValuesInBound;
		unsigned int rotateValuesMask = 0;

		for (int i = 0; i < (cpu.eu.instructionExtraData.directionIsLeft() ? rotateValuesLeft : rotateValuesInBound); ++i)
			rotateValuesMask |= 1 << i;

		const unsigned int resultUpperPartShift = (cpu.eu.instructionExtraData.directionIsLeft() ? rotateValuesInBound : rotateValuesLeft);
		const unsigned int resultUpperPart = (tempRresult & rotateValuesMask) << resultUpperPartShift;
		
		const unsigned int resultLowerPartShift = (cpu.eu.instructionExtraData.directionIsLeft() ? rotateValuesLeft : rotateValuesInBound);
		const unsigned int resultLowerPart = (tempRresult & ~rotateValuesMask) >> resultLowerPartShift;
		const unsigned int resultMSBMask = cpu.eu.operandSizeWord ? (1 << 16) : (1 << 8);
		unsigned int result = resultUpperPart | resultLowerPart;

		if (rotateCount != 0)
		{
			unsigned int newCarryFlagValues;
			if (cpu.eu.instructionExtraData.directionIsLeft())
				newCarryFlagValues = result & resultMSBMask;
			else
			{
				newCarryFlagValues = result & 1;
				if (cpu.eu.instructionExtraData.rotationWithCarry())
					result >>= 1;
			}
			cpu.updateStatusFlags(CPU::CARRY,newCarryFlagValues);
		}

		if (rotateCount == 1)
		{
			//Intel manuregs.al says that for ROR, OF = MSB(DEST) XOR MSB - 1(DEST)... WTF doregs.es it mean ?
			const unsigned int OFRresultMask = cpu.eu.instructionExtraData.directionIsLeft() ? resultMSBMask : 1;
			cpu.updateFlag(CPU::OVER,(result & OFRresultMask) ^ cpu.getFlagStatus(CPU::CARRY));
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

void AND	(void)
{
	const unsigned int result = Logic_twoOperandsInstruction([](unsigned int a, unsigned int b)
	{ return a & b; });
	
	cpu.clearFlags(CPU::OVER | CPU::CARRY);
	cpu.testSF(result);   cpu.testZF(result);   cpu.testPF(result);
}

void TEST	(void)
{
	const unsigned int result = Logic_twoOperandsInstruction<false>([](unsigned int a, unsigned int b)
	{ return a & b; });

	cpu.clearFlags(CPU::OVER | CPU::CARRY);
	cpu.testSF(result);   cpu.testZF(result);   cpu.testPF(result);
}

void OR		(void)
{
	const unsigned int result = Logic_twoOperandsInstruction([](unsigned int destValues, unsigned int srcValues){ return destValues | srcValues; });

	cpu.clearFlags(CPU::OVER | CPU::CARRY);
	cpu.testSF(result);   cpu.testZF(result);   cpu.testPF(result);
}

void XOR	(void)
{
	const unsigned int result = Logic_twoOperandsInstruction([](unsigned int destValues, unsigned int srcValues)
	{ return destValues ^ srcValues; });

	cpu.clearFlags(CPU::OVER | CPU::CARRY);
	cpu.testSF(result);   cpu.testZF(result);   cpu.testPF(result);
}
