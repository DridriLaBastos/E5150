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
			void readFromRAM(const unsigned int address);
			void writeToRAM(const unsigned int address, const uint8_t data);
			void readFromPort(const unsigned int address);
			void writeToPort(const unsigned int address, const uint8_t data);
			void instructionBufferQueuePop(const unsigned int n = 1);
		
			std::array<uint8_t, 5> instructionBufferQueue;
			unsigned int instructionBufferQueuePos;

		private:
			unsigned int mClockCountDown;
	};
}

#endif