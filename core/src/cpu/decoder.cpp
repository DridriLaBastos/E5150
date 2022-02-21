#include "decoder.hpp"
#include "decoderHelper.hpp"

using namespace E5150::I8086;
using namespace DECODER;

struct InstructionMetadata
{
	INSTRUCTION_PARTIAL_NAME firstByteInfo;
	uint8_t maxSize;
	uint8_t wModifier;
	uint8_t wModifySize;

	InstructionMetadata(const INSTRUCTION_PARTIAL_NAME info = INSTRUCTION_PARTIAL_NAME::INCOMPLETE, const uint8_t size = 4, const bool containsWModifier = false, const bool wModifierAffectsSize = false): firstByteInfo(info), maxSize(size), wModifier(containsWModifier), wModifySize(wModifierAffectsSize)
	{}
};

static std::array<InstructionMetadata,0xFF> firstByteDataLookupTable;
static REGISTER_NAME dataRegNameLookupTable [2][8] = {{ DECODER::REGISTER_NAME::AL, DECODER::REGISTER_NAME::CL, DECODER::REGISTER_NAME::DL, DECODER::REGISTER_NAME::BL, DECODER::REGISTER_NAME::AH, DECODER::REGISTER_NAME::CH, DECODER::REGISTER_NAME::BH, DECODER::REGISTER_NAME::DH }, { DECODER::REGISTER_NAME::AX, DECODER::REGISTER_NAME::CX, DECODER::REGISTER_NAME::DX, DECODER::REGISTER_NAME::BX, DECODER::REGISTER_NAME::SP, DECODER::REGISTER_NAME::BP, DECODER::REGISTER_NAME::SI, DECODER::REGISTER_NAME::DI }};
static REGISTER_NAME segRegNameLookupTavble [4] = { DECODER::REGISTER_NAME::ES, DECODER::REGISTER_NAME::CS, DECODER::REGISTER_NAME::DS, DECODER::REGISTER_NAME::SS };
static INSTRUCTION_STEP instructionStep = INSTRUCTION_STEP::FIRST_BYTE;

Decoder::Decoder()
{
	for (int i = 0; i <= 0b11; ++i) { firstByteDataLookupTable[0b10001000 | i] = { INSTRUCTION_PARTIAL_NAME::MOV_DW, 2, true }; }
	for (int i = 0; i <= 0b1111; ++i) { firstByteDataLookupTable[0b10110000 | i] = { INSTRUCTION_PARTIAL_NAME::MOV_W_REG, 3 }; }
	for (int i = 0; i <= 0b1; ++i) { firstByteDataLookupTable[0b10100000 | i] = { INSTRUCTION_PARTIAL_NAME::MOV_W1, 3, true }; }
	for (int i = 0; i <= 0b1; ++i) { firstByteDataLookupTable[0b10100010 | i] = { INSTRUCTION_PARTIAL_NAME::MOV_W2, 3 , true}; }

	for (int i = 0; i <= 0b111; ++i) { firstByteDataLookupTable[0b01010000 | i] = { INSTRUCTION_PARTIAL_NAME::PUSH_REG, 1 }; }
	for (int i = 0; i <= 0b11; ++i) { firstByteDataLookupTable[0b00110 | (i << 3)] = { INSTRUCTION_PARTIAL_NAME::PUSH_SEG, 1 }; }

	for (int i = 0; i <= 0b111; ++i) { firstByteDataLookupTable[0b01011000 | i] = { INSTRUCTION_PARTIAL_NAME::POP_REG, 1 }; }
	for (int i = 0; i <= 0b11; ++i) { firstByteDataLookupTable[0b00111 | (i << 3)] = { INSTRUCTION_PARTIAL_NAME::POP_SEG, 1 }; }

	for (int i = 0; i <= 0b1; ++i) { firstByteDataLookupTable[0b10000110 | i] = { INSTRUCTION_PARTIAL_NAME::XCHG_W, 2, true }; }
	for (int i = 0; i <= 0b111; ++i) { firstByteDataLookupTable[0b10010000 | i] = { INSTRUCTION_PARTIAL_NAME::XCHG_REG, 1 }; }

	for (int i = 0; i <= 0b1; ++i) { firstByteDataLookupTable[0b11100100 | i] = { INSTRUCTION_PARTIAL_NAME::IN_W1, 2, true }; }
	for (int i = 0; i <= 0b1; ++i) { firstByteDataLookupTable[0b11101100 | i] = { INSTRUCTION_PARTIAL_NAME::IN_W2, 1, true }; }

	for (int i = 0; i <= 0b1; ++i) { firstByteDataLookupTable[0b11100110 | i] = { INSTRUCTION_PARTIAL_NAME::OUT_W1, 2, true }; }
	for (int i = 0; i <= 0b1; ++i) { firstByteDataLookupTable[0b11101110 | i] = { INSTRUCTION_PARTIAL_NAME::OUT_W2, 1, true }; }

	firstByteDataLookupTable[0b11010111] = { INSTRUCTION_PARTIAL_NAME::XLAT, 2 };
	firstByteDataLookupTable[0b10001101] = { INSTRUCTION_PARTIAL_NAME::LEA, 2 };
	firstByteDataLookupTable[0b11000101] = { INSTRUCTION_PARTIAL_NAME::LDS, 2 };
	firstByteDataLookupTable[0b11000100] = { INSTRUCTION_PARTIAL_NAME::LES, 1 };
	firstByteDataLookupTable[0b10011111] = { INSTRUCTION_PARTIAL_NAME::LAHF, 1 };
	firstByteDataLookupTable[0b10011110] = { INSTRUCTION_PARTIAL_NAME::SAHF, 1 };
	firstByteDataLookupTable[0b10011100] = { INSTRUCTION_PARTIAL_NAME::PUSHF, 1 };
	firstByteDataLookupTable[0b10011101] = { INSTRUCTION_PARTIAL_NAME::POPF, 1 };
}

static void fillDFlagInfo (const bool dFlag, Decoder::DecodedInstruction& instruction)
{
	instruction.srcOperand = !dFlag ? DECODER::OPERAND::REGISTER : instruction.dstOperand;
	instruction.dstOperand = dFlag ? DECODER::OPERAND::REGISTER : instruction.dstOperand;
}

static unsigned int fillInstructionFirstByteInfo(const uint8_t instructionData, Decoder::DecodedInstruction& instruction)
{
	unsigned int instructionSize = 4;
	const DECODER::INSTRUCTION_PARTIAL_NAME patialName = firstByteDataLookupTable[instructionData].firstByteInfo;
	instructionStep = INSTRUCTION_STEP::MOD_REG_RM_BYTE;
	switch (patialName)
	{
	case DECODER::MOV_DW:
		instruction.name = DECODER::INSTRUCTION_NAME::MOV;
		instruction.wordOperand = instructionData & 0b1;
		fillDFlagInfo(instructionData & 0b01, instruction);
		return 2;
	
	case DECODER::MOV_W_REG:
		instruction.wordOperand = instructionData & 0b1000;
		instruction.srcOperand = DECODER::OPERAND::IMMEDIATE;
		instruction.dstOperand = DECODER::OPERAND::REGISTER;
		instruction.dstRegName = dataRegNameLookupTable[instruction.wordOperand][instructionData & 0b111];
		instructionStep = INSTRUCTION_STEP::DATA;
		return instruction.wordOperand ? 3 : 2;

	case DECODER::MOV_W1:
		instruction.wordOperand = instructionData & 0b1;
		instruction.srcOperand = DECODER::OPERAND::MEMORY;
		instruction.dstOperand = DECODER::OPERAND::REGISTER;
		instruction.dstRegName = dataRegNameLookupTable[instruction.wordOperand][0];
		instructionStep = INSTRUCTION_STEP::ADDR;
		return 3;

	case DECODER::MOV_W2:
		instruction.wordOperand = instructionData & 0b1;
		instruction.srcOperand = DECODER::OPERAND::MEMORY;
		instruction.dstOperand = DECODER::OPERAND::REGISTER;
		instruction.srcRegName = dataRegNameLookupTable[instruction.wordOperand][0];
		instructionStep = INSTRUCTION_STEP::ADDR;
		return 3;
	
	/* INCOMPLETE goes here */
	default:
		break;
	}
}

unsigned int fillIncompleteInstructionInfo(const uint8_t instructionData, Decoder::DecodedInstruction& instruction)
{
	return 0;
}

bool Decoder::feed(const uint8_t data)
{
	static unsigned int decodedInstructionByte = 1;
	static INSTRUCTION_PARTIAL_NAME partialInstructionName = INSTRUCTION_PARTIAL_NAME::INCOMPLETE;
	static unsigned int instructionSize;

	switch (instructionStep)
	{
		case INSTRUCTION_STEP::FIRST_BYTE:
			instructionSize = fillInstructionFirstByteInfo(data,decodedInstruction);
			break;
		
		case INSTRUCTION_STEP::INCOMPLETE_NEXT:

	}

	const bool instructionDecoded = instructionSize == firstByteDataLookupTable[data].maxSize;
	decodedInstructionByte = instructionDecoded ? 0 : (decodedInstructionByte + 1);
	return instructionDecoded;
}
