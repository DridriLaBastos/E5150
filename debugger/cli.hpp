#ifndef __DEBUGGER_GUI_HPP__
#define __DEBUGGER_GUI_HPP__

#include <string>

namespace E5150::DEBUGGER::CLI
{
	void ParseCommand(const std::string& cmdLine);
}


struct DebuggerCliFunctionPtr {
	void(*parseCommand)(const std::string& cmdLine);
};

#endif