#include <cassert>

#include "debugger/cli.hpp"
#include "debugger/debugger.hpp"

void E5150::DEBUGGER::CLI::ParseLine(const std::string& cmdLine)
{
	const auto cmdNameBegin = std::find_if_not(cmdLine.begin(), cmdLine.end(),[](const char c) {
		return isspace(c);
	});

	const auto cmdNameEnd = std::find_if(cmdNameBegin, cmdLine.end(),[](const char c) {
		return isspace(c);
	});

	const std::string cmdName (cmdNameBegin, cmdNameEnd);
	const std::string args (cmdNameEnd, cmdLine.end());
	const bool commandLaunched = DEBUGGER::Launch(cmdName, args);

	if (!commandLaunched) {
		fprintf(stderr,"ERROR: '%s' command not found\n",cmdName.c_str());
	}
}
