//
// Created by Adrien COURNAND on 19/05/2023.
//

#include "debugger/commands.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
/// Abstract command
////////////////////////////////////////////////////////////////////////////////////////////////

E5150::DEBUGGER::AbstractCommand::AbstractCommand(const std::string& n, const std::string& d): name(n), description(d), mApp(d,n)
{}

E5150::DEBUGGER::AbstractCommand::~AbstractCommand() = default;

void E5150::DEBUGGER::AbstractCommand::MakeReady() {}

void E5150::DEBUGGER::AbstractCommand::Prepare(const std::vector<std::string>& args)
{
	//CLI11 requires the arguments to be passed in reverse orders and to drop the name
	std::vector<std::string> cli11Args (args.rbegin(), args.rend()-1);

	try {
		MakeReady();
		mApp.parse(cli11Args);
		OnCommandParsed();

	} catch (const CLI::ParseError& e) {
		mApp.exit(e);
	}
}

void E5150::DEBUGGER::AbstractCommand::OnCommandParsed() {}

static unsigned int clockNumber = 0;
static unsigned int instructionNumber = 0;

E5150::DEBUGGER::COMMANDS::CommandContinue::CommandContinue() : AbstractCommand("continue", "Continue the execution of the emulation")
{
	auto clockOption = mApp.add_option("-c,--clock", clockNumber,"The number of clocks to continue");
	auto instrOption = mApp.add_option("-i,--instruction",instructionNumber,"The number of instruction to continue");
	clockOption->excludes(instrOption);
}

void E5150::DEBUGGER::COMMANDS::CommandContinue::MakeReady() {
	clockNumber = 0;
	instructionNumber = 0;
}

void E5150::DEBUGGER::COMMANDS::CommandContinue::OnCommandParsed()
{
	printf("continue for clock number: %d   instruction number: %d\n", clockNumber,instructionNumber);
}

bool E5150::DEBUGGER::COMMANDS::CommandContinue::Step(const bool instructionExecuted, const bool instructionDecoded)
{
	return false
}
