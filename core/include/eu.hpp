#ifndef __EU_HPP__
#define __EU_HPP__

namespace E5150::I8086
{
	class EU
	{
		public:
			EU(void);

			void clock(void);
		
		private:
			unsigned int mClockCountDown;
	};
}

#endif