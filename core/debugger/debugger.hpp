#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#define LOCK_FILE ".lock"

namespace E5150::DEBUGGER
{
	void init (void);
	void PrepareSimulationSide(void);
	void PrepareGuiSide(void);
	void clean (void);
	void wakeUp (const uint8_t instructionExecuted, const bool instructionDecoded);
	bool Launch(const std::string& commandName, const std::string& commandArgs);
}

#endif
