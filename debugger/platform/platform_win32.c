#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "../platform.h"

#include <io.h>
#include <fcntl.h>
#include <Windows.h>

const int FIFO_OPEN_RDONLY = _O_RDONLY;
const int FIFO_OPEN_WRONLY = _O_WRONLY;

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

static unsigned int mapInsert(const HANDLE const handle, const char* id)
{
	const int handleEntryPos = mapFindIndex(id);
	if (handleEntryPos >= 0)
	{
		return handleEntryPos;
	}

	mapCount += 1;
	map = realloc(map, sizeof(MapEntry)*mapCount);
	MapEntry* lastEntry = &map[mapCount - 1];
	lastEntry->id = malloc(strlen(id));
	strcpy(lastEntry->id,id);
	lastEntry->handle = handle;
	return mapCount - 1;
}

static void mapRemove(const unsigned int entry)
{
	if (entry >= mapCount) { return; }

	memcpy(&map[entry], &map[mapCount], sizeof(map[0]));
	mapCount -= 1;
}

//TODO: may benefit on some error checking
process_t processCreate(const char* processArgs[], const size_t processCommandLineArgsCount)
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

	//TODO: Why does WaitForSingleObject fails with timeout ?
	/*if (processCreationSucceed) {
		const DWORD waitStatus = WaitForSingleObject(pi.hProcess, INFINITE);

		processCreationSucceed = waitStatus == WAIT_OBJECT_0;
		
	}*/

	return processCreationSucceed ? pi.dwProcessId : -1;
}

enum PLATFORM_CODE processTerminate(const process_t process)
{
	BOOL b = GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, process);
	if (!b) { return PLATFORM_ERROR; }

	//TODO: Is it really necessary to call GetExitCode on window to clean the process ?
	HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, process);
	if (!h) { return PLATFORM_ERROR; }

	DWORD processExitCode;
	return GetExitCodeProcess(h, &processExitCode) ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

const char* errorGetDescription(void)
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

const uint64_t errorGetCode() { return GetLastError(); }

enum PLATFORM_CODE fifoCreate(const char* fifoFileName)
{
	COMPUTE_WIN32_PIPE_PATH(fifoFileName);
	//TODO: Not sure about that, seems to work.
	//What is the difference between MESSAGE_MODE and BYTE_MODE ? My (little) understanding is that with message the pipe want to read the n number of bytes we gives
	//in Write/Read File. With byte it's up to me to verify that I get the number of bytes I asked to send or receive.
	//What is the difference between PIPE_TYPE_MESSAGE with PIPE_READ_MODE message and PIPE_TYPE_MESSAGE with PIPE_READ_MODE message ?
	//
	//What I did for now is a pipe a byte ut with a readmode of message : it's still byte but we want to read the whole number of bytes sent... ? Is it good ? Bad ? Okay but stupid ?
	//I have no idea...
	const HANDLE namedPipeHandle = CreateNamedPipe(pipeSystemPath, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 256, 256, 0, NULL);

	if (namedPipeHandle == INVALID_HANDLE_VALUE) { return PLATFORM_ERROR; }

	mapInsert(namedPipeHandle, fifoFileName);
	return PLATFORM_SUCCESS;
}

int fifoOpen(const char* fifoFileName, const int openFlags)
{
	const int mapEntryIndex = mapFindIndex(fifoFileName);
	if (mapEntryIndex < 0) { return -1; }

	const HANDLE hFifo = map[mapEntryIndex].handle;
	const BOOL b = ConnectNamedPipe(hFifo, NULL);


	return _open_osfhandle(hFifo, openFlags);
}
