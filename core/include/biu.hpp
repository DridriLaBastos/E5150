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
			void requestNewFetchAddress (const uint16_t requestedCS, const uint16_t requestedIP);
		
			uint8_t EURequestReadByte (const unsigned int address);
			uint16_t EURequestReadWord (const unsigned int address);
			void EURequestWriteByte (const unsigned int address, const uint8_t data);
			void EURequestWriteWord (const unsigned int address, const uint16_t data);
			
			uint8_t EURequestINByte (const unsigned int address);
			uint16_t EURequestINWord (const unsigned int address);
			void EURequestOUTByte (const unsigned int address, const uint8_t data);
			void EURequestOUTWord (const unsigned int address, const uint16_t data);
		
			std::function<void(void)> clock;
			std::array<uint8_t, 5> instructionBufferQueue;
			unsigned int instructionBufferQueuePos;
			unsigned int EUDataAccessClockCountDown;
		
			uint16_t newCS;
			uint16_t newIP;
			bool newFetchAddressRequest;
	};
}

#endif
