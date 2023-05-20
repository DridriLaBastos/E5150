#include <cassert>

#include "debugger/cli.hpp"

std::vector<std::unique_ptr<E5150::DEBUGGER::Command>> E5150::DEBUGGER::CLI::commands;
E5150::DEBUGGER::Command* E5150::DEBUGGER::CLI::runningCommand = nullptr;

//TODO: Infinite loop when the entry something like '     "       ""'
std::vector<std::string> Tokenize(const std::string& str)
{
	std::vector<std::string> tokens;
	auto begin = str.begin();
	auto end = begin;

	do
	{
		begin = std::find_if_not(end, str.end(), [](const char c) {
			return isblank(c);
		});

		end = std::find_if(begin, str.end(), [](const char c) {
			return isblank(c);
		});

		if (begin != end)
		{ tokens.emplace_back(begin, end); }
	} while(end != str.end());

	return tokens;
}

void E5150::DEBUGGER::CLI::ParseLine(const std::string& cmdLine)
{
	const auto tokens = Tokenize(cmdLine);

	if (tokens.size() == 0)
	{ return; }

	const auto command = std::find_if(commands.begin(), commands.end(), [&tokens](const std::unique_ptr<Command>& commandPtr) {
		return commandPtr->name == tokens[0];
	});

	if (command == commands.end()) {
		printf("Unknown command '%s'\n", tokens[0].c_str());
		return;
	}

	runningCommand = command->get();
	runningCommand->Launch(tokens);
}

E5150::DEBUGGER::Command* E5150::DEBUGGER::CLI::GetRunningCommand() { return runningCommand; }

void E5150::DEBUGGER::CLI::CommandFinished(
#ifdef _DEBUG
											Command* cmd
#else
											void
#endif
)
{
	assert(cmd == runningCommand);
	runningCommand = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////
/// Abstract command
////////////////////////////////////////////////////////////////////////////////////////////////

E5150::DEBUGGER::Command::Command(const std::string& n, const std::string d): name(n), description(d), mParser(n)
{ mParser.add_description(d); }

E5150::DEBUGGER::Command::~Command()
{}

bool E5150::DEBUGGER::Command::GetExecutionStatus() { return mCommandIsRunning; }

void E5150::DEBUGGER::Command::Launch(const std::vector<std::string>& args)
{
	mCommandIsRunning = false;
	try {
		mParser.parse_args(args);
		mCommandIsRunning = InternalLaunch(args) == COMMAND_EXIT_SUCCESS_RUNNING;

	} catch (std::runtime_error e) {
		std::cerr << e.what() << "'\n";
		std::cerr << mParser << "\n";
	}
}

bool E5150::DEBUGGER::Command::Step(const bool instructionExecuted)
{
	mCommandIsRunning = InternalStep(instructionExecuted);

	if (!mCommandIsRunning) {
		CLI::CommandFinished(this);
	}
	return mCommandIsRunning;
}