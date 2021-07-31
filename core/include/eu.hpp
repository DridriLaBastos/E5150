#ifndef __EU_HPP__
#define __EU_HPP__

#include "util.hpp"

namespace E5150::I8086
{
	class EU
	{
		public:
			EU(void);

			void clock(void);
		
		private:
			unsigned int mClockCountDown;
			xed_decoded_inst_t decodedInst;
			const xed_inst_t* instruction;
			std::function<void(void)> mInstructionFunction;
	};
}

#endif