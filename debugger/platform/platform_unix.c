#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "../platform.h"

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

int fifoOpen(const char* fifoFileName, const int openFlags)
{ return open(fifoFileName,openFlags); }

const char* errorGetDescription(void) { return strerror(errno); }
const uint64_t errorGetCode(void) { return errno; }
