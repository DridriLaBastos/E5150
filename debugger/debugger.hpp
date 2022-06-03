#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

namespace E5150
{
	struct Debugger
	{
		static void init (void);
		static void deinit (void);
		static void wakeUp (const uint8_t instructionExecuted, const bool instructionDecoded);
		static void setClockLevelEmulatorInfo(const bool clockLevelInfo){ mClockLevelEmulatorInfo = clockLevelInfo; }
		template<typename ...Args>
		static void CLOCK_LEVEL_EMULATOR_INFO(const Args&&... args)
		{
			#ifdef DEBUGGER
				if (mClockLevelEmulatorInfo)
					printf(std::forward<Args>(args)...);
			#endif
		};

		private:
			static bool mClockLevelEmulatorInfo;
	};
}

#endif
