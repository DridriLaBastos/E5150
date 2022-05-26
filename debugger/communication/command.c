#include <stdbool.h>
#include <stdio.h>

#include "command.h"
#include "communication.h"

static int sendCommandToEmulatorAndGetStatus(const COMMAND_TYPE commandType)
{
	static COMMAND_RECEIVED_STATUS commandReceivedStatus;

	writeToEmulator(&commandType, sizeof(COMMAND_TYPE));
	readFromEmulator(&commandReceivedStatus, sizeof(COMMAND_RECEIVED_STATUS));

	return commandReceivedStatus;
}

///////////////////////////////////////////////////////////////////////////////////////////
////                                CONTINUE
///////////////////////////////////////////////////////////////////////////////////////////

static int debuggerToEmulator_SendContinueCommandInfo(const CONTINUE_TYPE continueType, const unsigned int value)
{
	if (!sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_CONTINUE))
	{ return false; }

	writeToEmulator(&continueType,sizeof(CONTINUE_TYPE));
	writeToEmulator(&value, sizeof(unsigned int));
	return true;
}

int sendContinueCommandInfo(const int instructionCounts, const int clockCounts, const int busCycleCounts)
{
	if (instructionCounts >= 0) { return debuggerToEmulator_SendContinueCommandInfo(CONTINUE_TYPE_INSTRUCTION, instructionCounts); }
	if (clockCounts >= 0) { return debuggerToEmulator_SendContinueCommandInfo(CONTINUE_TYPE_CLOCK, clockCounts); }
	if (busCycleCounts >= 0) { return debuggerToEmulator_SendContinueCommandInfo(CONTINUE_TYPE_BUS, busCycleCounts); }
	return debuggerToEmulator_SendContinueCommandInfo(CONTINUE_TYPE_INFINITE, -1);
}

int sendStepCommandInfo(void)
{
	return sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_STEP);
}

int sendDisplayCommandInfo(const int toggleFlags, const int toggleInstructions, const int toggleRegisters, const int changeLogLevel)
{
	sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_DISPLAY);
	// writeToEmulator(&toggleFlags, sizeof(toggleFlags));
	// writeToEmulator(&toggleInstructions, sizeof(toggleInstructions));
	// writeToEmulator(&toggleRegisters, sizeof(toggleRegisters));
	// writeToEmulator(&changeLogLevel, sizeof(changeLogLevel));
	return false;
}
