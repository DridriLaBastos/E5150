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

			unsigned int getEAComputationClockCount();
			unsigned int EAAddress;
			bool newFetchAddress;

			xed_decoded_inst_t decodedInst;
			const xed_inst_t* xedInst;
			uint16_t newCS;
			uint16_t newIP;
	};
}

#endif
