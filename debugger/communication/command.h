#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include <stdint.h>

#define COMMUNICATION_TEST_VALUE 0xFFAABBCC

typedef enum : uint8_t
{
	COMMAND_RECEIVED_FAILURE = 0,
	COMMAND_RECEIVED_SUCCESS
} COMMAND_RECEIVED_STATUS;

typedef enum : uint8_t
{
	COMMAND_TYPE_CONTINUE=0,
	COMMAND_TYPE_STEP,
	COMMAND_TYPE_DISPLAY,
	COMMAND_TYPE_ERROR,
}COMMAND_TYPE;

typedef enum : uint8_t
{ CONTINUE_TYPE_CLOCK, CONTINUE_TYPE_INSTRUCTION, CONTINUE_TYPE_BUS, CONTINUE_TYPE_INFINITE } CONTINUE_TYPE;

/**
 * @brief Register the file descriptor to talk between the emulator and the debugger
 * 
 * @param toDebugger descriptor for writing from the emulator to the debugger
 * @param toEmulator descriptor for writing from the debugger to the emulator
 */
void registerCommunicationFifo(const int fromEmulator, const int toEmulator);

int sendContinueCommandInfo(const int instructionCounts, const int clocCounts, const int busCycleCounts);
int sendStepCommandInfo(void);
int sendDsiplayCommandInfo(void);

#endif//__COMMAND_HPP__