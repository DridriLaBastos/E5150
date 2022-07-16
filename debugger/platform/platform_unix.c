#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syslimits.h>
#include <stdio.h>

#include "../platform.h"

const int FIFO_OPEN_RDONLY = O_RDONLY;
const int FIFO_OPEN_WRONLY = O_WRONLY;

process_t processCreate(const char* processCommandLineArgs[], const size_t processCommandLineArgsCount)
{
	const pid_t createdPID = fork();

	if (createdPID == 0)
	{
		//We are in child process
		char** execFunctionArgs = alloca(sizeof(char*) * (processCommandLineArgsCount + 1));

		execFunctionArgs[processCommandLineArgsCount] = NULL;

		for (int i = 0; i < processCommandLineArgsCount; ++i)
		{ execFunctionArgs[i] = processCommandLineArgs[i]; }

		execvp(execFunctionArgs[0], execFunctionArgs);
		//TODO: How to notify the emulator that the creation of the debugger failed ? Maybe using signal SIGUSR1/2?
		exit(127);
	}
	return createdPID;
}

enum PLATFORM_CODE processTerminate(process_t process)
{
	if (kill(process,SIGTERM) < 0) { return PLATFORM_ERROR; }
	return waitpid(process,NULL,WUNTRACED) == process ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

enum PLATFORM_CODE fifoCreate (const char* fifoFileName)
{
	const int fifo = mkfifo(fifoFileName,S_IRWXG|S_IRWXO|S_IRWXU);

	if (fifo >= 0) { return PLATFORM_SUCCESS; }

	const int fifoAlreadyExists = errno == EEXIST;

	return fifoAlreadyExists ? PLATFORM_FIFO_ALREADY_CREATED : PLATFORM_ERROR;
}

fifo_t fifoOpen(const char* fifoFileName, const int openFlags)
{ return open(fifoFileName,openFlags); }

enum PLATFORM_CODE fifoClose(const fifo_t fifo)
{
	char fifoPath [PATH_MAX];
	if(fcntl(fifo,F_GETPATH,fifoPath) < 0) { return PLATFORM_ERROR; }

	return remove(fifoPath) == 0 ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

enum PLATFORM_CODE fifoWrite(const fifo_t fifo, const void* data, const size_t noctet)
{ return write(fifo,data,noctet) == noctet ? PLATFORM_SUCCESS : PLATFORM_ERROR; }

enum PLATFORM_CODE fifoRead(const fifo_t fifo, void* const buf, const size_t noctet)
{ return read(fifo,buf,noctet) == noctet ? PLATFORM_SUCCESS : PLATFORM_ERROR; }

const char* errorGetDescription(void) { return strerror(errno); }
const uint64_t errorGetCode(void) { return errno; }
