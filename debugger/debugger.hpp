#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include "debugger/cli.hpp"

namespace E5150::DEBUGGER
{
	void init (void);
	void clean (void);
	void wakeUp (const uint8_t instructionExecuted, const bool instructionDecoded);
	bool getDebuggerIsRunningState(void);

	DebuggerCliFunctionPtr& GetCliFunctionPtr(void);

	struct DebuggerState {
		std::vector<std::string> output;
		bool commandAdded;
	};
}

#endif
