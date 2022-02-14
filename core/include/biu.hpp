#ifndef __BIU_HPP__
#define __BIU_HPP__

#include "util.hpp"

namespace E5150::I8086
{
	class BIU
	{
		public:
			enum class WORKING_MODE 
			{
				FETCH_INSTRUCTION,
				FETCH_DATA,
				WAIT_ROOM_IN_QUEUE,
				WAIT_END_OF_INTERRUPT_DATA_SAVE_SEQUENCE
			};
			struct InternalInfos
			{
				unsigned int BUS_CYCLE_CLOCK = 4;
				unsigned int BUS_CYCLE_CLOCK_LEFT = BUS_CYCLE_CLOCK;
				unsigned int EU_DATA_ACCESS_CLOCK_LEFT = 0;
				bool CPU_EXECUTING_INTERRUPT_DATASAVE_SEQUENCE = false;
				bool UPDATE_WORKING_MODE = false;
				BIU::WORKING_MODE workingMode = BIU::WORKING_MODE::FETCH_INSTRUCTION;
				unsigned int IP_OFFSET = 0;
			};

		public:
			void updateClockFunction(void);
			void instructionBufferQueuePop(const unsigned int n);
			void startInterruptDataSaveSequence (void);
			void endInterruptDataSaveSequence   (void);
			void endControlTransferInstruction (const bool didJmp = true);
			void IPToNextInstruction(const unsigned int instructionLength);
			void clock(void);

			uint8_t readByte (const unsigned int address) const;
			uint16_t readWord (const unsigned int address) const;

			void writeByte (const unsigned int address, const uint8_t byte) const;
			void writeWord (const unsigned int address, const uint16_t word) const;

			void requestMemoryByte(const unsigned int nBytes) noexcept;
			
			uint8_t inByte (const unsigned int address);
			uint16_t inWord (const unsigned int address);
			void outByte (const unsigned int address, const uint8_t data);
			void outWord (const unsigned int address, const uint16_t data);

			static const InternalInfos& getDebugWorkingState (void);

			std::array<uint8_t, 5> instructionBufferQueue;
			unsigned int instructionBufferQueuePos;
	};
}

#endif
