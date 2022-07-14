#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "../platform.h"

#include <Windows.h>

const int FIFO_OPEN_RDONLY = 0;
const int FIFO_OPEN_WRONLY = 1;

static const char* namedPipeSystemPath = "\\\\.\\pipe\\";

//+1 for the last NULL character
#define COMPUTE_WIN32_PIPE_PATH(pipeName)	const size_t pipeSystemPathLength = strlen(pipeName) + strlen(namedPipeSystemPath) + 1;\
											const char* pipeSystemPath = _malloca(pipeSystemPathLength);\
											strcpy(pipeSystemPath,namedPipeSystemPath);\
											strncat(pipeSystemPath,pipeName,pipeSystemPathLength)
#define ERROR_MESSAGE_MAX_SIZE 256

typedef struct MapEntry_t {
	const char* id;
	HANDLE handle;
} MapEntry;

static MapEntry* map = NULL;
static unsigned int mapCount = 0;

static int mapFindIndex(const char* id)
{
	for (int i = 0; i < mapCount; ++i)
	{
		if (strcmp(id, map[i].id) == 0) { return i; }
	}

	return -1;
}

static unsigned int mapInsert(const HANDLE* const handle, const char* id)
{
	const int handleEntryPos = mapFindIndex(id);
	if (handleEntryPos >= 0)
	{
		return handleEntryPos;
	}

	mapCount += 1;
	map = realloc(map, sizeof(map[0])*mapCount);
	MapEntry* lastEntry = &map[mapCount - 1];
	lastEntry->id = malloc(strlen(id));
	strcpy(lastEntry->id,id);
	memcpy(handle, &lastEntry->handle,sizeof(HANDLE));
	return mapCount - 1;
}

static void mapRemove(const unsigned int entry)
{
	if (entry >= mapCount) { return; }

	memcpy(&map[entry], &map[mapCount], sizeof(map[0]));
	mapCount -= 1;
}

//TODO: may benefit on some error checking
enum PLATFORM_CODE processCreate(const char* processArgs[], const size_t processCommandLineArgsCount)
{
	//For each arguments for the process we want to add a space inbetween, this creates processCommandLineArgsCount - 1 spaces.
	//We add 1 more to the size for the last null character
	//Then we add the size of each arguments of the process to create the final size of the process launch command line
	size_t processCommandLineSize = processCommandLineArgsCount;

	for (int i = 0; i < processCommandLineArgsCount; ++i)
	{
		processCommandLineSize += strlen(processArgs[i]);
	}

	char* processLaunchCommandLine = _malloca(processCommandLineSize);
	ZeroMemory(processLaunchCommandLine, processCommandLineSize);

	size_t processCmdIndex = 0;

	for (int i = 0; i < processCommandLineArgsCount; ++i)
	{
		strcpy(processLaunchCommandLine + processCmdIndex, processArgs[i]);
		processCmdIndex += strlen(processArgs[i]);
		processLaunchCommandLine[processCmdIndex++] = ' ';
	}
	processLaunchCommandLine[processCommandLineSize - 1] = '\0';

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));

	si.cb = sizeof(si);

	BOOL processCreationSucceed = CreateProcess(NULL, processLaunchCommandLine, NULL, NULL, FALSE, 0, NULL, NULL,&si,&pi);

	if (processCreationSucceed) {
		const DWORD waitStatus = WaitForSingleObject(pi.hProcess, 3000);

		processCreationSucceed = waitStatus == WAIT_OBJECT_0;
	}

	return processCreationSucceed ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

enum PLATFORM_CODE processWait(const process_t process) {}
enum PLATFORM_CODE processKill(const process_t process) {}
int platformGetProcessID(const process_t process) {}

enum PLATFORM_CODE fifoCreate(const char* fifoFileName)
{
	COMPUTE_WIN32_PIPE_PATH(fifoFileName);
	const HANDLE namedPipeHandle = CreateNamedPipe(pipeSystemPath, PIPE_ACCESS_DUPLEX | FILE_FLAG_WRITE_THROUGH, PIPE_TYPE_BYTE | PIPE_REJECT_REMOTE_CLIENTS, 1, 0, 0, 0, NULL);
	mapInsert(&namedPipeHandle, fifoFileName);
	return namedPipeHandle == INVALID_HANDLE_VALUE ? PLATFORM_ERROR : PLATFORM_SUCCESS;
}

const char* platformGetErrorDescription(void)
{
	static char messageDescription[ERROR_MESSAGE_MAX_SIZE];

	const DWORD lastError = GetLastError();
	const DWORD messageLength = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, lastError, LANG_SYSTEM_DEFAULT, messageDescription, ERROR_MESSAGE_MAX_SIZE, NULL);

	for (int i = 0; i < messageLength; ++i)
	{
		if (messageDescription[i] == '\r')
		{
			messageDescription[i] = '\0';
			break;
		}
	}

	if (messageLength < 0)
	{
		snprintf(messageDescription, ERROR_MESSAGE_MAX_SIZE, "Cannot retrieve message description for error code %d", lastError);
	}
	return messageDescription;
}

fifo_t fifoOpen(const char* fifoFileName, const int openFlags)
{
	return mapFindIndex(fifoFileName);
}

enum PLATFORM_CODE fifoClose(const fifo_t fifo)
{
	
}

enum PLATFORM_CODE fifoRemove(const char* fifoFilename) {}

enum PLATFORM_CODE fifoWrite(const fifo_t fifo, const void* data, const size_t noctet)
{
	if (fifo < 0) { return PLATFORM_ERROR; }
	const HANDLE fifoHandle = map[fifo].handle;
	DWORD dataWritten = 0;
	const BOOL writeOk = WriteFile(fifoHandle, data, noctet, &dataWritten, NULL);

	return writeOk ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

enum PLATFORM_CODE fifoRead(const fifo_t fifo, void* const buf, const size_t noctet)
{
	if (fifo < 0) { return PLATFORM_ERROR; }
	const HANDLE fifoHandle = map[fifo].handle;
	DWORD dataRead = 0;
	const BOOL writeOk = ReadFile(fifoHandle, buf, noctet, &dataRead, NULL);

	return writeOk ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}
