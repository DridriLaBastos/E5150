#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "command.h"

static int fromEmulator = -1;
static int toEmulator = -1;

void registerCommunicationFifos (const int _fromEmulator, const int _toEmulator)
{
	fromEmulator = _fromEmulator;
	toEmulator = _toEmulator;
}

static int sendCommandToEmulatorAndGetStatus(const COMMAND_TYPE commandType)
{
	static COMMAND_RECEIVED_STATUS commandReceivedStatus;

	write(toEmulator, &commandType, sizeof(COMMAND_TYPE));
	read(fromEmulator,&commandReceivedStatus, sizeof(COMMAND_RECEIVED_STATUS));

	return commandReceivedStatus;
}

///////////////////////////////////////////////////////////////////////////////////////////
////                                CONTINUE
///////////////////////////////////////////////////////////////////////////////////////////

static int debuggerToEmulator_SendContinueCommandInfo(const CONTINUE_TYPE continueType, const unsigned int value)
{
	if (!sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_CONTINUE))
	{ return false; }

	//write(toEmulator, &continueType,sizeof(CONTINUE_TYPE));
	//write(toEmulator, &value, sizeof(unsigned int));
	return true;
}

int sendContinueCommandInfo(const int instructionCounts, const int clockCounts, const int busCycleCounts)
{
	/*if (instructionCounts >= 0) { return debuggerToEmulator_SendContinueCommandInfo(CONTINUE_TYPE_INSTRUCTION, instructionCounts); }
	if (clockCounts >= 0) { return debuggerToEmulator_SendContinueCommandInfo(CONTINUE_TYPE_CLOCK, clockCounts); }
	if (busCycleCounts >= 0) { return debuggerToEmulator_SendContinueCommandInfo(CONTINUE_TYPE_BUS, busCycleCounts); }
	return debuggerToEmulator_SendContinueCommandInfo(CONTINUE_TYPE_INFINITE, -1);*/
	return sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_CONTINUE);
}

int sendStepCommandInfo(void)
{
	return sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_STEP);
}

int sendDisplayCommandInfo(void)
{
	sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_DISPLAY);
	return false;
}
