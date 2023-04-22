#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <vector>

#include "core/8086.hpp"
#include "communication/command.h"

namespace E5150::DEBUGGER
{
	enum class DEBUGGER_STD_STREAM {
		STDOUT, STDERR
	};
	void init (void);
	void clean (void);
	void wakeUp (const uint8_t instructionExecuted, const bool instructionDecoded);
	void sendCommand(void);
	bool getDebuggerIsRunningState(void);
	FILE* getDebuggerStdStream(DEBUGGER_STD_STREAM stream);

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
			CPU* i8086;
			bool debuggerIsRunning;
		};

		void init(void);
		//void draw(void);
		const State getState(void);
		void clean(void);
	}
}

#endif
