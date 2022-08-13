#include <stdbool.h>

#include "command.h"
#include "communication.h"

static COMMAND_RECEIVED_STATUS sendCommandToEmulatorAndGetStatus(const COMMAND_TYPE commandType)
{
	static COMMAND_RECEIVED_STATUS commandReceivedStatus;

	WRITE_TO_EMULATOR(&commandType, sizeof(commandType));
	READ_FROM_EMULATOR(&commandReceivedStatus, sizeof(COMMAND_RECEIVED_STATUS));

	return commandReceivedStatus;
}

///////////////////////////////////////////////////////////////////////////////////////////
////                                CONTINUE
///////////////////////////////////////////////////////////////////////////////////////////

int sendContinueCommandInfo(const int instructionCounts, const int clockCounts)
{
	const COMMAND_RECEIVED_STATUS status = sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_CONTINUE);
	if (status) { return false; }

	PASS_TYPE passType;
	unsigned int passCount;

	if (instructionCounts > 0)
	{
		passType = PASS_TYPE_INSTRUCTIONS;
		passCount = instructionCounts;
	}
	else if (clockCounts > 0)
	{
		passType = PASS_TYPE_CLOCKS;
		passCount = clockCounts;
	}
	else
	{
		passType = PASS_TYPE_INFINITE;
		passCount = -1;
	}

	WRITE_TO_EMULATOR(&passType, sizeof(passType));
	WRITE_TO_EMULATOR(&passCount, sizeof(passCount));
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
////                                  STEP
///////////////////////////////////////////////////////////////////////////////////////////

int sendStepCommandInfo(const int instructionFlag, const int clockFlag, const int passFlag)
{
	const COMMAND_RECEIVED_STATUS status = sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_STEP);
	if (status) { return false; }

	uint8_t stepFlags = 0;

	if (instructionFlag) { stepFlags |= PASS_TYPE_INSTRUCTIONS; }
	if (clockFlag) { stepFlags |= PASS_TYPE_CLOCKS; }
	if (passFlag) { stepFlags |= PASS_TYPE_STEP_THROUGH; }

	WRITE_TO_EMULATOR(&stepFlags,sizeof(stepFlags));
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
////                                DISPLAY
///////////////////////////////////////////////////////////////////////////////////////////

int sendDisplayCommandInfo(const int newLogLevel)
{
	sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_DISPLAY);
	WRITE_TO_EMULATOR(&newLogLevel, sizeof(newLogLevel));
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
////                                QUIT
///////////////////////////////////////////////////////////////////////////////////////////
int sendQuitCommandInfo()
{
	sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_QUIT);
	return true;
}
