#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

namespace E5150::DEBUGGER
{
	constexpr char LOCK_FILE [] = ".lock";
	void init (void);
	void PrepareSimulationSide(void);
	void PrepareGuiSide(void);
	void clean (void);
	void wakeUp (const uint8_t instructionExecuted, const bool instructionDecoded);
	bool Launch(const std::string& commandName, const std::string& commandArgs);
}

#endif
