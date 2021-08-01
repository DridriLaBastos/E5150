#ifndef __EU_HPP__
#define __EU_HPP__

#include "util.hpp"
#include "instructions.hpp"
namespace E5150::I8086
{
	class EU
	{
		public:
			EU(void);

			bool clock(void);

			uint16_t pop (void);
			void push (const uint16_t data);

			void farCall (const uint16_t seg, const uint16_t offset);
			void farRet (void);
		
			unsigned int clockCountDown;
			xed_decoded_inst_t decodedInst;
			std::function<unsigned int(void)> instructionGetClockCount;
			std::function<void(void)> instructionExec;

			unsigned int getEAComputationClockCount();
			const xed_inst_t* xedInst;

			bool newFetchAddress;
			uint16_t newCS;
			uint16_t newIP;
	};
}

#endif
