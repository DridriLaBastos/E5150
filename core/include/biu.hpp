#ifndef __BIU_HPP__
#define __BIU_HPP__

#include "util.hpp"

namespace E5150::I8086
{
	class BIU
	{
		public:
			BIU(void);
			void clock(void);
			uint8_t readByte (const unsigned int address);
			uint16_t readWord (const unsigned int address);
			void writeByte(const unsigned int address, const uint8_t data);
			void writeWord(const unsigned int address, const uint16_t data);
			
			uint8_t inByte (const unsigned int address);
			uint16_t inWord (const unsigned int address);
			void outByte(const unsigned int address, const uint8_t data);
			void outWord(const unsigned int address, const uint16_t data);
		
			std::array<uint8_t, 5> instructionBufferQueue;
			unsigned int instructionBufferQueuePos;

		private:
			unsigned int mClockCountDown;
	};
}

#endif
