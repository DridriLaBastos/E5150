//
// Created by Adrien COURNAND on 26/03/2023.
//

#ifndef E5150_COMMUNICATION_H
#define E5150_COMMUNICATION_H

#include "platform/platform.h"

#define EMULATOR_TO_DEBUGGER_FIFO_FILENAME ".ed.fifo"
#define DEBUGGER_TO_EMULATOR_FIFO_FILENAME ".de.fifo"
#define COMMUNICATION_TEST_VALUE 0x1A2B3C4D

typedef enum {
	DECOM_STATUS_OK = 0,
	DECOM_STATUS_CONFIGURE_UNKNOWN,
	DECOM_STATUS_ED_CHANNEL_INIT_ERROR,
	DECOM_STATUS_DE_CHANNEL_INIT_ERROR,
	DECOM_STATUS_ED_CHANNEL_REMOVE_ERROR,
	DECOM_STATUS_DE_CHANNEL_REMOVE_ERROR,
	DECOM_STATUS_NULL_CHANNEL,
	DECOM_STATUS_NULL_BUFFER,
	DECOM_STATUS_ERRNO,
} DECOM_STATUS;

///Who is calling a function (emulator or debugger)
typedef enum {
	DECOM_CONFIGURE_DEBUGGER,
	DECOM_CONFIGURE_EMULATOR,
}DECOM_CONFIGURE;

///In what direction goes the communication ?
typedef enum {
	DECOM_DIRECTION_EMULATOR_TO_DEBUGGER,
	DECOM_DIRECTION_DEBUGGER_TO_EMULATOR
} DECOM_DIRECTION;

#ifdef __cplusplus
extern "C" {
#endif

	//TODO: API consideration : should this function called makefifo from the platform code ?
	//For now it is the responsibility of the calling code to be sure that the file exists

	DLL_EXPORT DECOM_STATUS decom_Prepare(const DECOM_CONFIGURE configure);

	/**
	 *
	 * @param configure Which side of the configuration is initialized
	 * @return calling status
	 */
	DLL_EXPORT DECOM_STATUS decom_InitCommunication(const DECOM_CONFIGURE configure);

	DLL_EXPORT DECOM_STATUS decom_ReadFromChannel(const DECOM_DIRECTION direction, void* readBuffer, const size_t readBufferSizeInOctets, size_t* const ignoredIfNull_ReadCount);
	DLL_EXPORT DECOM_STATUS decom_WriteToChannel(const DECOM_DIRECTION direction, void* writeBuffer, const size_t writeBufferSizeInOctets, size_t* const ignoredIfNull_WriteCount);

	DLL_EXPORT DECOM_STATUS decom_TestConnection(const DECOM_CONFIGURE channelConfigure,unsigned int* ignoredIfNull_TestValue);

	DLL_EXPORT DECOM_STATUS decom_SafeCloseChannel(void);
	DLL_EXPORT DECOM_STATUS decom_CleanChannelArtifacts(void);

	DLL_EXPORT const char* decom_GetLastErrorDescription(void);

#define READ_FROM_EMULATOR(readBufferPtr, readBufferSizeInOctets) decom_ReadFromChannel(DECOM_DIRECTION_EMULATOR_TO_DEBUGGER,readBufferPtr,readBufferSizeInOctets,NULL)
#define WRITE_TO_EMULATOR(writeBufferPtr, writeBufferSizeInOctets) decom_WriteToChannel(DECOM_DIRECTION_DEBUGGER_TO_EMULATOR,writeBufferPtr,writeBufferSizeInOctets,NULL)

#define READ_FROM_DEBUGGER(readBufferPtr, readBufferSizeInOctets) decom_ReadFromChannel(DECOM_DIRECTION_DEBUGGER_TO_EMULATOR,readBufferPtr,readBufferSizeInOctets,NULL)
#define WRITE_TO_DEBUGGER(writeBufferPtr, writeBufferSizeInOctets) decom_WriteToChannel(DECOM_DIRECTION_EMULATOR_TO_DEBUGGER,writeBufferPtr,writeBufferSizeInOctets,NULL)

#ifdef __cplusplus
}
#endif

#endif