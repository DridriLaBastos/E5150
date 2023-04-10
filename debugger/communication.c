//
// Created by Adrien COURNAND on 29/03/2023.
//

#include <errno.h>
#include <string.h>

#include "platform/platform.h"
#include "communication.h"

static FILE* channelDebuggerToEmulator = NULL;
static FILE* channelEmulatorToDebugger = NULL;

static const char* const statusDescriptionNoError = "No error";
static const char* const statusDescriptionUnknownStatus = "An error happened that has not been registered : this is a bug";
static const char* const statusDescriptionNullChannel = "io function called on a null channel";
static const char* const statusDescriptionNullBuffer = "buffer given for io function is null";
static const char* lastErrorDescription = statusDescriptionNoError;

static DECOM_STATUS configureError(const DECOM_STATUS status)
{
	switch(status)
	{
		case DECOM_STATUS_OK:
			lastErrorDescription = statusDescriptionNoError; break;
		case DECOM_STATUS_NULL_CHANNEL:
			lastErrorDescription = statusDescriptionNullChannel; break;
		case DECOM_STATUS_ERRNO:
			lastErrorDescription = strerror(errno); break;
		case DECOM_STATUS_NULL_BUFFER:
			lastErrorDescription = statusDescriptionNullBuffer; break;
		default:
			lastErrorDescription = statusDescriptionUnknownStatus; break;
	}
	return status;
}

const char* const decom_GetLastErrorDescription()
{
	const char* const desc = lastErrorDescription;
	lastErrorDescription = statusDescriptionNoError;
	return desc;
}

DECOM_STATUS decom_InitCommunication(const DECOM_CONFIGURE configure)
{
	//TODO: How pertinent is the use of Yoda syntax here ?
	if ((configure != DECOM_CONFIGURE_DEBUGGER) && (configure != DECOM_CONFIGURE_EMULATOR))
	{
		return configureError(DECOM_STATUS_CONFIGURE_UNKNOWN);
	}

	if (channelEmulatorToDebugger)
	{
		if(fclose(channelEmulatorToDebugger))
		{
			fprintf(stderr,"Error while closing '%s' - ERRNO %d = '%s'\n",EMULATOR_TO_DEBUGGER_FIFO_FILENAME, errno,
			        strerror((errno)));
			channelEmulatorToDebugger = NULL;
		}
	}

	if (channelDebuggerToEmulator)
	{
		if(fclose(channelDebuggerToEmulator))
		{
			fprintf(stderr,"Error while closing '%s' - ERRNO %d = '%s'\n",DEBUGGER_TO_EMULATOR_FIFO_FILENAME, errno,
			        strerror((errno)));
			channelDebuggerToEmulator = NULL;
		}
	}

	const char* const openModeEmulatorToDebugger = (configure == DECOM_CONFIGURE_EMULATOR) ? "w" : "r";
	const char* const openModeDebuggerToEmulator = (configure == DECOM_CONFIGURE_DEBUGGER) ? "w" : "r";

	channelEmulatorToDebugger = fopen(EMULATOR_TO_DEBUGGER_FIFO_FILENAME, openModeEmulatorToDebugger);
	channelDebuggerToEmulator = fopen(DEBUGGER_TO_EMULATOR_FIFO_FILENAME, openModeDebuggerToEmulator);

	if (!(channelDebuggerToEmulator && channelEmulatorToDebugger))
	{
		configureError(DECOM_STATUS_ERRNO);
		return !channelEmulatorToDebugger ? DECOM_STATUS_ED_CHANNEL_INIT_ERROR : DECOM_STATUS_DE_CHANNEL_INIT_ERROR;
	}

	//TODO: profile the read and write to a channel
	//The lib c doesn't provide a way to do unbuffered IO. So instead of configuring the stream
	//as unbuffered I let them buffered but will use fflush after each write operation.
	//I should measure what are the performance implication of that
	//setbuf(channelEmulatorToDebugger,"U");
	//setbuf(channelDebuggerToEmulator,"U");

	return DECOM_STATUS_OK;
}

#define CHECK_SAFE_MACRO_FUNCTION(FAIL_CONDITIONS,DECOM_STATUS_ON_FAIL) do { if(FAIL_CONDITIONS) { return configureError(DECOM_STATUS_ON_FAIL); } } while(0)
#define CHECK_NON_NULL_PTR(ptr, checkFailedError) CHECK_SAFE_MACRO_FUNCTION(!ptr,checkFailedError)
#define CHECK_DECOMCONFIG_IS_CHANNEL(config) CHECK_SAFE_MACRO_FUNCTION((config != DECOM_CONFIGURE_DEBUGGER) && (config != DECOM_CONFIGURE_EMULATOR),DECOM_STATUS_CONFIGURE_UNKNOWN)
#define CHECK_DECOMCONFIG_IS_CHANNEL_DIRECTION(config) CHECK_SAFE_MACRO_FUNCTION((config != DECOM_CONFIGURE_EMULATOR_TO_DEBUGGER) && (config != DECOM_CONFIGURE_DEBUGGER_TO_EMULATOR),DECOM_STATUS_CONFIGURE_UNKNOWN)

DECOM_STATUS decom_ReadFromChannel(const DECOM_CONFIGURE channelConfigure, void* readBuffer, const size_t readBufferSizeInOctets, size_t* const ignoredIfNull_ReadCount)
{
	CHECK_DECOMCONFIG_IS_CHANNEL_DIRECTION(channelConfigure);
	CHECK_NON_NULL_PTR(readBuffer, DECOM_STATUS_NULL_BUFFER);

	FILE* const channel = (channelConfigure == DECOM_CONFIGURE_EMULATOR_TO_DEBUGGER) ? channelEmulatorToDebugger : channelDebuggerToEmulator;
	CHECK_NON_NULL_PTR(channel, DECOM_STATUS_NULL_CHANNEL);

	const size_t ret = fread(readBuffer,readBufferSizeInOctets,1,channel);

	if (ignoredIfNull_ReadCount) { *ignoredIfNull_ReadCount = ret; }

	if(ferror(channel)) {
		return configureError(DECOM_STATUS_ERRNO);
	}

	return DECOM_STATUS_OK;
}

DECOM_STATUS decom_WriteToChannel(const DECOM_CONFIGURE channelConfigure, void* writeBuffer, const size_t writeBufferSizeInOctets, size_t* const ignoredIfNull_WriteCount)
{
	CHECK_DECOMCONFIG_IS_CHANNEL_DIRECTION(channelConfigure);
	CHECK_NON_NULL_PTR(writeBuffer, DECOM_STATUS_NULL_BUFFER);

	FILE* const channel = (channelConfigure == DECOM_CONFIGURE_EMULATOR_TO_DEBUGGER) ? channelEmulatorToDebugger : channelDebuggerToEmulator;
	CHECK_NON_NULL_PTR(channel, DECOM_STATUS_NULL_CHANNEL);

	const size_t ret = fwrite(writeBuffer,writeBufferSizeInOctets,1,channel);
	fflush(channel);

	if (ignoredIfNull_WriteCount) { *ignoredIfNull_WriteCount = ret; }

	if(ferror(channel)) {
		return configureError(DECOM_STATUS_ERRNO);
	}

	return DECOM_STATUS_OK;
}

DECOM_STATUS decom_TestConnection(const DECOM_CONFIGURE channelConfigure, unsigned int* ignoredIfNull_TestValue)
{
	CHECK_DECOMCONFIG_IS_CHANNEL(channelConfigure);
	CHECK_NON_NULL_PTR(channelEmulatorToDebugger,DECOM_STATUS_NULL_CHANNEL);

	unsigned int testValue;

	if (channelConfigure == DECOM_CONFIGURE_EMULATOR)
	{
		testValue = COMMUNICATION_TEST_VALUE;
		fwrite(&testValue,sizeof(testValue),1,channelEmulatorToDebugger);
		fflush(channelEmulatorToDebugger);
		//write(fileno(channelEmulatorToDebugger),&testValue,sizeof(testValue));
	}
	else
	{
		fread(&testValue,sizeof(testValue),1,channelEmulatorToDebugger);
		if (ignoredIfNull_TestValue)
		{ *ignoredIfNull_TestValue = testValue; }
	}

	if (ferror(channelEmulatorToDebugger))
	{ return configureError(DECOM_STATUS_ERRNO); }
	return DECOM_STATUS_OK;
}

DECOM_STATUS decom_SafeCloseChannel()
{
	if (channelEmulatorToDebugger)
	{ fclose(channelEmulatorToDebugger); }

	if (channelDebuggerToEmulator)
	{ fclose(channelDebuggerToEmulator); }

	channelEmulatorToDebugger = NULL;
	channelDebuggerToEmulator = NULL;
	return DECOM_STATUS_OK;
}

DECOM_STATUS decom_CleanChannelArtifacts(void)
{
	const int edRemoveStatus = remove(EMULATOR_TO_DEBUGGER_FIFO_FILENAME);
	const int deRemoveStatus = remove(DEBUGGER_TO_EMULATOR_FIFO_FILENAME);

	if (edRemoveStatus | deRemoveStatus)
	{
		configureError(DECOM_STATUS_ERRNO);
	}

	return !(edRemoveStatus | deRemoveStatus) ? DECOM_STATUS_OK : (edRemoveStatus ? DECOM_STATUS_ED_CHANNEL_REMOVE_ERROR : DECOM_STATUS_DE_CHANNEL_REMOVE_ERROR);
}
