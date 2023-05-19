#include <iostream>

#include "debugger/cli.hpp"

void E5150::DEBUGGER::CLI::ParseCommand(const std::string& cmdLine)
{
	std::cout << "Added command '" << cmdLine << "'\n";
}