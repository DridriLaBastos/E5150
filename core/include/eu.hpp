#ifndef __EU_HPP__
#define __EU_HPP__

#include "util.hpp"
#include "instructions.hpp"
namespace E5150::I8086
{
	class EU
	{
		public:
			union InstructionExtraData_t
			{
				bool isSigned;
				bool withCarry;
				uint8_t Shift_RotateFlags[sizeof(bool)];

				void setDirectionIsLeft(void) { Shift_RotateFlags[0] |= 0b1; }
				bool directionIsLeft(void) const { return Shift_RotateFlags[0] & 0b1; }

				void setInstructionIsArithmetic(void) { Shift_RotateFlags[0] |= 0b10; }
				bool instructionIsArithmetic(void) const { return Shift_RotateFlags[0] & 0b10; }

				void setRotationWithCarry(void) { Shift_RotateFlags[0] |= 0b100; }
				bool rotationWithCarry(void) const { return Shift_RotateFlags[0] & 0b100; }
			};
		public:
			EU(void);

			uint16_t pop (void);
			void push (const uint16_t data);
			void updateClockFunction(void);

			void farCall (const uint16_t seg, const uint16_t offset);
			void farRet (void);

			unsigned int computeEAAndGetComputationClockCount();
			unsigned int EAAddress;
			unsigned int repeatCount;
			unsigned int instructionLength;

			xed_decoded_inst_t decodedInst;
			const xed_inst_t* xedInst;
			bool operandSizeWord;
			bool repInstructionFinished;
			bool (*clock)(void);
			InstructionExtraData_t instructionExtraData;
	};
}

#endif
