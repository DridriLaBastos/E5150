#ifndef __EU_HPP__
#define __EU_HPP__

#include "util.hpp"
#include "decoder.hpp"
#include "instructions.hpp"
namespace E5150::I8086
{
	class EU
	{
		public:
			enum class WORKING_MODE
			{
				EXEC_INTERRUPT_SERVICE_PROCEDURE,
				EXEC_INSTRUCTION,
				EXEC_REP_INSTRUCTION,
				DECODE
			};

			struct InternalInfos
			{
				WORKING_MODE EUWorkingMode = WORKING_MODE::DECODE;
				WORKING_MODE EUNextWorkingMode = EUWorkingMode;
				unsigned int INSTRUCTION_CLOCK_LEFT = 0;
				unsigned int REP_COUNT = 0;
				unsigned int CURRENT_INSTRUCTION_CS = 0;
				unsigned int CURRENT_INSTRUCTION_IP = 0;
			};

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
				
				void clearData(void) { isSigned = 0; }
			};

		public:
			void enterInterruptServiceProcedure(const unsigned int procedureClockCycles);
			bool clock(void);

			void farCall (const uint16_t seg, const uint16_t offset);
			void farRet (void);

			static const InternalInfos& getDebugWorkingState (void);

			unsigned int computeEAAndGetComputationClockCount();
			unsigned int EAAddress;
			unsigned int repeatCount;
			unsigned int instructionLength;

			xed_decoded_inst_t decodedInst;
			const xed_inst_t* xedInst;
			bool operandSizeWord;
			bool repInstructionFinished;
			InstructionExtraData_t instructionExtraData;
			Decoder mDecoder;

	};
}

#endif
