#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <vector>

#include "communication/command.h"

namespace E5150::Debugger
{
	void init (void);
	void clean (void);
	void wakeUp (const uint8_t instructionExecuted, const bool instructionDecoded);
	void sendCommand(void);

	namespace GUI {
		enum class CONSOLE_ENTRY_TYPE
		{ COMMAND, DEBUGGER_STDERR, DEBUGGER_STDOUT };

		struct ConsoleEntry
		{
			const CONSOLE_ENTRY_TYPE type;
			const std::string prefix;
			const std::string str;

			ConsoleEntry(const CONSOLE_ENTRY_TYPE t, const std::string& s, const std::string& p = ""): type(t), prefix(p), str(s) {}
		};

		struct State
		{
			std::mutex* debugConsoleMutex;
			std::vector<ConsoleEntry>* debugConsoleEntries;
		};

		void init(void);
		//void draw(void);
		const State getState(void);
		void clean(void);
	}
}

#endif
