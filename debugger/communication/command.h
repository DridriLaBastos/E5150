#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include <stdint.h>

#include "platform.h"

typedef enum
{
	COMMAND_RECEIVED_FAILURE = 0,
	COMMAND_RECEIVED_SUCCESS
} COMMAND_RECEIVED_STATUS;

typedef enum
{
	COMMAND_TYPE_CONTINUE=0,
	COMMAND_TYPE_STEP,
	COMMAND_TYPE_DISPLAY,
	COMMAND_TYPE_ERROR,
	COMMAND_TYPE_QUIT
}COMMAND_TYPE;

typedef enum
{ CONTINUE_TYPE_CLOCK, CONTINUE_TYPE_INSTRUCTION, CONTINUE_TYPE_INFINITE } CONTINUE_TYPE;

typedef enum
{ DISPLAY_TYPE_INSTRUCTIONS, DISPLAY_TYPE_REGISTERS, DISPLAY_TYPE_FLAGS, DISPLAY_TYPE_LOGLEVEL } DISPLAY_TYPE;

typedef enum
{
	STEP_TYPE_INSTRUCTION = 1 << 0,
	STEP_TYPE_CLOCK = 1 << 1,
	STEP_TYPE_PASS = 1 << 2
} STEP_TYPE;

DLL_EXPORT int sendContinueCommandInfo(const int instructionCounts, const int clockCounts);
DLL_EXPORT int sendStepCommandInfo(const int instructionFlag, const int clockFlag, const int passFlag);
DLL_EXPORT int sendDisplayCommandInfo(const int toggleFlags, const int toggleInstructions, const int toggleRegisters, const int changeLogLevel);
DLL_EXPORT int sendQuitCommandInfo(void);

#endif//__COMMAND_HPP__