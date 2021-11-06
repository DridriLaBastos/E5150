#ifndef __BIU_HPP__
#define __BIU_HPP__

#include "util.hpp"

namespace E5150::I8086
{
	class BIU
	{
		public:
			BIU(void);
			void updateClockFunction(void);
			void instructionBufferQueuePop(const unsigned int n);
			void resetInstructionBufferQueue(void);
			void startControlTransferInstruction (void);
			void endControlTransferInstruction (const bool flushInstructionBuffer = false);

			uint8_t readByte (const unsigned int address) const;
			uint16_t readWord (const unsigned int address) const;

			void writeByte (const unsigned int address, const uint8_t byte) const;
			void writeWord (const unsigned int address, const uint16_t word) const;
		
			/*uint8_t EURequestReadByte (const unsigned int address);
			uint16_t EURequestReadWord (const unsigned int address);
			void EURequestWriteByte (const unsigned int address, const uint8_t data);
			void EURequestWriteWord (const unsigned int address, const uint16_t data);*/

			void requestMemoryByte(const unsigned int nBytes) noexcept;
			
			uint8_t inByte (const unsigned int address);
			uint16_t inWord (const unsigned int address);
			void outByte (const unsigned int address, const uint8_t data);
			void outWord (const unsigned int address, const uint16_t data);
		
			void(*clock)(void);
			std::array<uint8_t, 5> instructionBufferQueue;
			unsigned int instructionBufferQueuePos;
	};
}

#endif
