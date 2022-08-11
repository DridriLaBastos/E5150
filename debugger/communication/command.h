#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include <stdint.h>

#include "debugger/platform.h"

/**
 * This file contains the C declarations of the functions called from the debugger to send
 * command information to the emulator.
 *
 * Each functions return 0 if the command should the debugger cli runs (for example after a display command we still
 * want the debugger to ask the user for a command), or non 0 value (usually 1) if the command should stop the debugger cli
 * (for example after a step or a continue command).
 *
 * The commands are separated in two categories : pass commands and non-pass commands.
 *
 * Pass commands are the commands that should let the emulation continue after their execution. The debugger expects the return
 * value of the execution of those command to be non 0. Example of pass commands are step or continue. When one of those commands
 * are requested, the emulation should continue for the amount of time requested (clock or instructions).
 * When a pass command returns a 0 value, the debugger should consider that an error happen during the execution of the command and
 * should let the emulation continue.
 *
 * The quit command is a special case : whether an error happens or not, if the emulator received the quit command it will always quit
 * even if the command returns a non-zero value.
 */

typedef enum
{
	COMMAND_RECEIVED_SUCCESS,
	COMMAND_RECEIVED_FAILURE
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
	PASS_TYPE_STEP_THROUGH	= 1 << 3
} PASS_TYPE;

typedef enum
{ DISPLAY_TYPE_INSTRUCTIONS, DISPLAY_TYPE_REGISTERS, DISPLAY_TYPE_FLAGS, DISPLAY_TYPE_LOGLEVEL } DISPLAY_TYPE;

DLL_EXPORT int sendContinueCommandInfo(const int instructionCounts, const int clockCounts);
DLL_EXPORT int sendStepCommandInfo(const int instructionFlag, const int clockFlag, const int passFlag);
DLL_EXPORT int sendDisplayCommandInfo(const int newLogLevel);
DLL_EXPORT int sendQuitCommandInfo(void);

#endif//__COMMAND_HPP__