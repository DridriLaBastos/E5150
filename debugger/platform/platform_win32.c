#include <stdio.h>
#include <malloc.h>

#include "../platform.h"

#include <Windows.h>

const int FIFO_OPEN_RDONLY = 0;
const int FIFO_OPEN_WRONLY = 1;

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

enum PLATFORM_CODE fifoCreate(const char* fifoFileName) {}
const char* platformGetErrorDescription(void)
{
	static const char* noStr = "No description available for error code on windows for now";
	return noStr;
}
fifo_t fifoOpen(const char* fifoFileName, const int openFlags) {}

enum PLATFORM_CODE fifoClose(const fifo_t fifo) {}
enum PLATFORM_CODE fifoRemove(const char* fifoFilename) {}
enum PLATFORM_CODE fifoWrite(const fifo_t fifo, const void* data, const size_t noctet) {}
enum PLATFORM_CODE fifoRead(const fifo_t fifo, void* const buf, const size_t noctet) {}
