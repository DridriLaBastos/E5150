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

static void sendPassCommandToEmulator(const PassCommandInfo* const info)
{ WRITE_TO_EMULATOR(info, sizeof(PassCommandInfo)); }

///////////////////////////////////////////////////////////////////////////////////////////
////                                CONTINUE
///////////////////////////////////////////////////////////////////////////////////////////

int sendContinueCommandInfo(const int instructionCounts, const int clockCounts)
{
	const COMMAND_RECEIVED_STATUS status = sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_CONTINUE);
	if (status) { return false; }

	PassCommandInfo info;

	PASS_TYPE passType;
	unsigned int passCount;

	if (instructionCounts > 0)
	{
		info.passType = PASS_TYPE_INSTRUCTIONS;
		info.instructionCount = instructionCounts;
	}
	else if (clockCounts > 0)
	{
		info.passType = PASS_TYPE_CLOCKS;
		info.instructionCount = clockCounts;
	}
	else
	{
		info.passType = PASS_TYPE_INFINITE;
		info.passCount = 0;
	}
	sendPassCommandToEmulator(&info);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
////                                  STEP
///////////////////////////////////////////////////////////////////////////////////////////

int sendStepCommandInfo(const int instructionFlag, const int clockFlag, const int passFlag)
{
	const COMMAND_RECEIVED_STATUS status = sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_STEP);
	if (status) { return false; }

	PassCommandInfo info; info.passCount = 1;

	if (instructionFlag) { info.passType = PASS_TYPE_INSTRUCTIONS; }
	if (clockFlag) { info.passType = PASS_TYPE_CLOCKS; }
	if (passFlag) { info.passType = PASS_TYPE_INSTRUCTION_STEP_THROUGH; }

	sendPassCommandToEmulator(&info);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
////                                DISPLAY
///////////////////////////////////////////////////////////////////////////////////////////

int sendDisplayCommandInfo(const int newLogLevel)
{
	sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_DISPLAY);
	WRITE_TO_EMULATOR(&(DisplayCommandInfo) { .newLogLevel=newLogLevel }, sizeof(DisplayCommandInfo));
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
