#ifndef __COMMANDS_HPP__
#define __COMMANDS_HPP__

#include "debugger/debugger.hpp"

namespace E5150::DEBUGGER::COMMANDS
{
	class CommandContinue: public Command
	{
	public:
		CommandContinue(void);
		bool Step(const bool instructionExecuted, const bool instructionDecoded) final;
	};
}

#endif