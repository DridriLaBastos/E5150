#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Register the file descriptor to talk between the emulator and the debugger
 * 
 * @param toDebugger descriptor for writing from the emulator to the debugger
 * @param toEmulator descriptor for writing from the debugger to the emulator
 */
void registerCommunicationFifos(const int fromEmulator, const int toEmulator);

/**
 * @brief Platform independant function to write data to the emulator
 * 
 * @param indata pointer to the data to write
 * @param size size in octets of the data to write
 */
void writeToEmulator(const uint8_t* const indata, const size_t size);

/**
 * @brief Platform independant function to read data from the emulator
 * 
 * @param outdata 
 * @param size 
 */
void readFromEmulator(uint8_t* const outdata, const size_t size);

#endif