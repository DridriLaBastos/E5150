#ifndef __BIU_HPP__
#define __BIU_HPP__

#include "util.hpp"

namespace E5150::I8086
{
	class BIU
	{
		public:
			BIU(void);
			void instructionBufferQueuePop(const unsigned int n);
			void resetInstructionBufferQueue(void);
			void startControlTransferInstruction (void);
			void endControlTransferInstruction (void);

			uint8_t readByte (const unsigned int address) const;
			uint16_t readWord (const unsigned int address) const;

			void writeByte (const unsigned int address, const uint8_t byte) const;
			void writeWord (const unsigned int address, const uint16_t word) const;
		
			/*uint8_t EURequestReadByte (const unsigned int address);
			uint16_t EURequestReadWord (const unsigned int address);
			void EURequestWriteByte (const unsigned int address, const uint8_t data);
			void EURequestWriteWord (const unsigned int address, const uint16_t data);*/

			void requestMemoryByte(const unsigned int nBytes) noexcept;
			
			uint8_t EURequestINByte (const unsigned int address);
			uint16_t EURequestINWord (const unsigned int address);
			void EURequestOUTByte (const unsigned int address, const uint8_t data);
			void EURequestOUTWord (const unsigned int address, const uint16_t data);
		
			void(*clock)(void);
			void(*nextClockFunction)(void);
			std::array<uint8_t, 5> instructionBufferQueue;
			unsigned int instructionBufferQueuePos;
			unsigned int EUMemoryAccessClockCountDown;
	};
}

#endif
