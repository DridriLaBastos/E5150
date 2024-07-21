#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#define LOCK_FILE ".lock"

#include "platform/platform.h"

namespace E5150::DEBUGGER {
#if 0
	void init (void);
	void PrepareSimulationSide(void);
	void PrepareGuiSide(void);
	void clean (void);
	void wakeUp (const uint8_t instructionExecuted, const bool instructionDecoded);
	bool Launch(const std::string& commandName, const std::string& commandArgs);
#endif
	void Init(void);
	void Clean(void);
	void WakeUp(const unsigned int cpuEvents);
	void ParseCmdLine(std::string line);
}

#endif
