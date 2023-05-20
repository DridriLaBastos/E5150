//
// Created by Adrien COURNAND on 19/05/2023.
//

#include "debugger/commands.hpp"

E5150::DEBUGGER::CommandContinue::CommandContinue() : Command("continue", "Continue the execution of the emulation")
{}

E5150::DEBUGGER::COMMAND_LAUNCH_RETURN E5150::DEBUGGER::CommandContinue::InternalLaunch(const std::vector<std::string>& args)
{
	return E5150::DEBUGGER::COMMAND_EXIT_SUCCESS_NO_RUNNING;
}

bool E5150::DEBUGGER::CommandContinue::InternalStep(const bool instructionExecuted)
{
	return false;
}
