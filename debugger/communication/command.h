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

typedef enum : uint8_t
{ DISPLAY_TYPE_INSTRUCTIONS, DISPLAY_TYPE_REGISTERS, DISPLAY_TYPE_FLAGS, DISPLAY_TYPE_LOGLEVEL } DISPLAY_TYPE;

int sendContinueCommandInfo(const int instructionCounts, const int clocCounts, const int busCycleCounts);
int sendStepCommandInfo(void);
int sendDisplayCommandInfo(const int toggleFlags, const int toggleInstructions, const int toggleRegisters, const int changeLogLevel);

#endif//__COMMAND_HPP__