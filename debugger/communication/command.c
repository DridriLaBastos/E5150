#include <stdbool.h>
#include <stdio.h>

#include "command.h"
#include "communication.h"

static COMMAND_RECEIVED_STATUS sendCommandToEmulatorAndGetStatus(const COMMAND_TYPE commandType)
{
	static COMMAND_RECEIVED_STATUS commandReceivedStatus;

	writeToEmulator(&commandType, sizeof(commandType));
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

	writeToEmulator(&continueType,sizeof(continueType));
	writeToEmulator(&value, sizeof(value));
	return true;
}

int sendContinueCommandInfo(const int instructionCounts, const int clockCounts)
{
	if (instructionCounts >= 0) { return debuggerToEmulator_SendContinueCommandInfo(CONTINUE_TYPE_INSTRUCTION, instructionCounts); }
	if (clockCounts >= 0) { return debuggerToEmulator_SendContinueCommandInfo(CONTINUE_TYPE_CLOCK, clockCounts); }
	return debuggerToEmulator_SendContinueCommandInfo(CONTINUE_TYPE_INFINITE, -1);
}

///////////////////////////////////////////////////////////////////////////////////////////
////                                  STEP
///////////////////////////////////////////////////////////////////////////////////////////

int sendStepCommandInfo(const int instructionFlag, const int clockFlag, const int passFlag)
{
	if(sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_STEP))
	{
		uint8_t stepFlags = 0;

		if (instructionFlag) { stepFlags |= STEP_TYPE_INSTRUCTION; }
		if (clockFlag) { stepFlags |= STEP_TYPE_CLOCK; }
		if (passFlag) { stepFlags |= STEP_TYPE_PASS; }

		writeToEmulator(&stepFlags,1);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
////                                DISPLAY
///////////////////////////////////////////////////////////////////////////////////////////

int sendDisplayCommandInfo(const int toggleFlags, const int toggleInstructions, const int toggleRegisters, const int changeLogLevel)
{
	sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_DISPLAY);
	writeToEmulator(&toggleFlags, sizeof(toggleFlags));
	writeToEmulator(&toggleInstructions, sizeof(toggleInstructions));
	writeToEmulator(&toggleRegisters, sizeof(toggleRegisters));
	writeToEmulator(&changeLogLevel, sizeof(changeLogLevel));
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
////                                QUIT
///////////////////////////////////////////////////////////////////////////////////////////
int sendQuitCommandInfo()
{
	sendCommandToEmulatorAndGetStatus(COMMAND_TYPE_QUIT);
}
