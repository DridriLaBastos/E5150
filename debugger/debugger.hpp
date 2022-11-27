#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include "communication/command.h"

namespace E5150::Debugger
{
	void init (void);
	void deinit (void);
	void wakeUp (const uint8_t instructionExecuted, const bool instructionDecoded);

	namespace GUI {
		void drawConsole(void);
	}
}

#endif
