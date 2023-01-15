#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <vector>

#include "communication/command.h"
#include "gui/common/gui_states.hpp"

namespace E5150::Debugger
{
	void init (void);
	void clean (void);
	void wakeUp (const uint8_t instructionExecuted, const bool instructionDecoded);

	namespace GUI {
		void init(void);
		void draw(void);
		void populateDebuggerGUIState(DebuggerGUIState* const state);
		void clean(void);

		enum class CONSOLE_ENTRY_TYPE
		{ COMMAND, DEBUGGER_STDERR, DEBUGGER_STDOUT };

		using ConsoleEntries = std::vector<std::pair<CONSOLE_ENTRY_TYPE,std::string>>;
	}
}

#endif
