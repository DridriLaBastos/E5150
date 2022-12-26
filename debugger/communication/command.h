#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include <stdint.h>

#include "platform/platform.h"

/**
 * This file contains the C declarations of the functions called from the debugger to send command information and
 * configuration to the emulator.
 *
 * The functions declared in this file are called from the debugger. The return value of the function will dictate
 * the debugger the value to send to the emulator. If the function returns à 0, the same value is sent to the debugger.
 * Any other value will be translated to a 1.
 *
 * The command associated to each functions are divided into two groups : pass-command and non-pass command.
 *
 * A non-pass command will block forever the emulation and get back to the debugger cli when the function called to
 * performs its execution ends. The display command is such a command. At the end of the execution of the command, the
 * debugger prompt should be displayed again.
 *
 * A pass command will instruct the emulator to run the simulation for a certain amount of time (or endlessly). After the
 * end of the execution of the function associated to the command, the debugger will let the emulator running.
 * The continue or step commands are such command. When the function associated to the command ends its
 * execution, the emulator should run the amount of time specified and only after ask for the debugger prompt.
 *
 * Functions associated to non-pass command should always return non 0. Functions associated to a pass-command should return
 * non 0 on error and 0 for success.
 *
 * The quit command is a special case : whether the function returns a 0 or not, the emulator will always quit and kill
 * the debugger.
 */

typedef enum
{
	COMMAND_RECEIVED_SUCCESS = 1,
	COMMAND_RECEIVED_FAILURE = 0
} COMMAND_RECEIVED_STATUS;

typedef enum
{
	COMMAND_TYPE_CONTINUE=0,
	COMMAND_TYPE_STEP,
	COMMAND_TYPE_DISPLAY,
	COMMAND_TYPE_ERROR,
	COMMAND_TYPE_QUIT
}COMMAND_TYPE;

typedef enum {
	PASS_TYPE_INFINITE 		= 1 << 0,
	PASS_TYPE_INSTRUCTIONS	= 1 << 1,
	PASS_TYPE_CLOCKS		= 1 << 2,
	PASS_TYPE_INSTRUCTION_STEP_THROUGH	= 1 << 3
} PASS_TYPE;

typedef enum
{ DISPLAY_TYPE_INSTRUCTIONS, DISPLAY_TYPE_REGISTERS, DISPLAY_TYPE_FLAGS, DISPLAY_TYPE_LOGLEVEL } DISPLAY_TYPE;

typedef struct {
	union {
		unsigned int instructionCount;
		unsigned int clockCounts;
		unsigned int passCount;
	};
	PASS_TYPE passType;
} PassCommandInfo;

typedef struct {
	unsigned int newLogLevel;
} DisplayCommandInfo;

DLL_EXPORT int sendContinueCommandInfo(const int instructionCounts, const int clockCounts);
DLL_EXPORT int sendStepCommandInfo(const int instructionFlag, const int clockFlag, const int passFlag);
DLL_EXPORT int sendDisplayCommandInfo(const int newLogLevel);
DLL_EXPORT int sendQuitCommandInfo(void);

#endif//__COMMAND_HPP__