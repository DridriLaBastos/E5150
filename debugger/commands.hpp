#ifndef __COMMANDS_HPP__
#define __COMMANDS_HPP__

#include "debugger/cli.hpp"

namespace E5150::DEBUGGER
{
	class CommandContinue: public Command
	{
	public:
		CommandContinue(void);
		COMMAND_LAUNCH_RETURN InternalLaunch(const std::vector<std::string>& args) final;
		bool InternalStep(const bool instructionExecuted) final;
	};
}

#endif