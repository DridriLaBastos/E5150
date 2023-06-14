//
// Created by Adrien COURNAND on 19/05/2023.
//

#include "debugger/commands.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
/// Abstract command
////////////////////////////////////////////////////////////////////////////////////////////////

E5150::DEBUGGER::AbstractCommand::AbstractCommand(const std::string& n, const std::string& d): name(n), description(d)
{}

E5150::DEBUGGER::AbstractCommand::~AbstractCommand() = default;

bool E5150::DEBUGGER::AbstractCommand::Parse(const std::vector<std::string>& args)
{
	//CLI11 requires the arguments to be passed in reverse orders and to drop the name
	std::vector<std::string> argv (args.rbegin(), args.rend()-1);

	CLI::App app (description, name);

	try {
		InternalParse(app,argv);
	}
	catch (const CLI::ParseError& e) {
		app.exit(e);
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////
/// Continue command
////////////////////////////////////////////////////////////////////////////////////////////////

E5150::DEBUGGER::COMMANDS::CommandContinue::CommandContinue() :
AbstractCommand("continue","Continue the execution of the emulation, if no options are provided, "
						         "the emulation continues indefinitely")
{}

void E5150::DEBUGGER::COMMANDS::CommandContinue::InternalParse(CLI::App &app, std::vector<std::string> &argv)
{
	unsigned int clockNumber = 0;
	unsigned int instructionNumber = 0;
	CLI::Option* optionClock = app.add_option("-c,--clock", clockNumber,
												 "The number of clocks that shouls be emulated");
	CLI::Option* optionInstructions = app.add_option("-i,--instructions", instructionNumber,
													 "The number of instruction to emulate");

	optionClock->excludes(optionInstructions);

	app.parse(argv);

	const bool infinite = (clockNumber == 0) && (instructionNumber == 0);
	printf("#clock cycles: %d   #instructions: %d   infinite: %d\n", clockNumber, instructionNumber, infinite);
}

bool E5150::DEBUGGER::COMMANDS::CommandContinue::Step(const bool instructionExecuted, const bool instructionDecoded)
{
#if 0
	printf("continue for #clock %d   #instruction %d\n", clockNumber, instructionNumber);
	clockNumber -= clockNumber > 0;
	instructionNumber -= (instructionNumber > 0) && instructionExecuted;

	return !(clockNumber || instructionNumber || infiniteContinue);
#endif
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////
/// Step command
////////////////////////////////////////////////////////////////////////////////////////////////
E5150::DEBUGGER::COMMANDS::CommandStep::CommandStep(): AbstractCommand("step",
																	   "Continue the emulation for the specified amount of time")
{}


void E5150::DEBUGGER::COMMANDS::CommandStep::InternalParse(CLI::App &app, std::vector<std::string> &argv)
{
	unsigned int clockNumber = 0;
	unsigned int instructionNumber = 0;
	bool follow = false;
	CLI::Option* optionClock = app.add_option("-c,--clock", clockNumber,
	                                          "The number of clock cycles to pass");
	CLI::Option* optionInstructions = app.add_option("-i,--instructions", instructionNumber,
	                                                 "The number of instructions to pass");

	CLI::Option* flagFollow = app.add_flag("-f,--follow", follow,
										   "When specified the debugger follows control transfers instructions");

	optionClock->excludes(optionInstructions);


	app.parse(argv);

	printf("#clock cycles: %d   #instructions: %d   %s\n", clockNumber, instructionNumber, follow ? "[FOLLOW]" : "");
}

bool E5150::DEBUGGER::COMMANDS::CommandStep::Step(const bool instructionExecuted, const bool instructionDecoded)
{
#if 0
	printf("continue for #clock %d   #instruction %d\n", clockNumber, instructionNumber);
	clockNumber -= clockNumber > 0;
	instructionNumber -= (instructionNumber > 0) && instructionExecuted;

	return !(clockNumber || instructionNumber || infiniteContinue);
#endif
	return true;
}