#include <cassert>

#include "debugger/cli.hpp"
#include "debugger/debugger.hpp"

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

	const bool commandLaunched = DEBUGGER::Launch(tokens[0], tokens);

	if (!commandLaunched) {
		fprintf(stderr,"ERROR: '%s' command not found\n",cmdLine.c_str());
	}
}
