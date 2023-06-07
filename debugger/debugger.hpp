#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

namespace E5150::DEBUGGER
{
	void init (void);
	void clean (void);
	void wakeUp (const uint8_t instructionExecuted, const bool instructionDecoded);
	bool Launch(const std::string &commandName, std::vector<std::string>& argv);
}

#endif
