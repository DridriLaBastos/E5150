//
// Created by Adrien COURNAND on 19/05/2023.
//

#include "debugger/commands.hpp"

E5150::DEBUGGER::COMMANDS::CommandContinue::CommandContinue() : Command("continue", "Continue the execution of the emulation")
{}

bool E5150::DEBUGGER::COMMANDS::CommandContinue::Step(const bool instructionExecuted, const bool instructionDecoded)
{
	return false;
}
