#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>

#include "../platform.h"

int FIFO_OPEN_RDONLY = O_RDONLY;
int FIFO_OPEN_WRONLY = O_WRONLY;

enum PLATFORM_CODE processCreate(const char* processCommandLineArgs[], const size_t processCommandLineArgsCount)
{
	const pid_t createdPID = fork();

	if (createdPID > 0)
	{
		//We are in child process
		char** execFunctionArgs = alloca(sizeof(char*) * (processCommandLineArgsCount + 1));

		execFunctionArgs[processCommandLineArgsCount] = NULL;

		for (int i = 0; i < processCommandLineArgsCount; ++i)
		{ execFunctionArgs[i] = processCommandLineArgs[i]; }

		execvp(execFunctionArgs[0], execFunctionArgs);
		printf("[PLATFORM ERROR]: Cannot launch debugger: '%s'\n", strerror(errno));
		exit(127);
	}

	return createdPID;
}

enum PLATFORM_CODE processWait(const process_t process)
{
	int childRet;
	return waitpid(process,NULL,WUNTRACED) == process ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

enum PLATFORM_CODE processKill(const process_t process)
{ return kill(process,SIGKILL) ? PLATFORM_ERROR : PLATFORM_SUCCESS; }

enum PLATFORM_CODE fifoCreate (const char* fifoFileName)
{
	const int fifo = mkfifo(fifoFileName,S_IRWXG|S_IRWXO|S_IRWXU);

	if (fifo >= 0) { return PLATFORM_SUCCESS; }

	const int fifoAlreadyExists = errno == EEXIST;

	return fifoAlreadyExists ? PLATFORM_FIFO_ALREADY_CREATED : PLATFORM_ERROR;
}

const char* platformGetErrorDescription(void) { return strerror(errno); }

fifo_t fifoOpen(const char* fifoFileName, const int openFlags)
{ return open(fifoFileName,openFlags); }

enum PLATFORM_CODE fifoClose(const fifo_t fifo) { return close(fifo) == 0 ? PLATFORM_SUCCESS : PLATFORM_ERROR; }

enum PLATFORM_CODE fifoWrite(const fifo_t fifo, const void* data, const size_t noctet)
{ return write(fifo,data,noctet) == noctet ? PLATFORM_SUCCESS : PLATFORM_ERROR; }

enum PLATFORM_CODE fifoRead(const fifo_t fifo, void* const buf, const size_t noctet)
{ return read(fifo,buf,noctet) == noctet ? PLATFORM_SUCCESS : PLATFORM_ERROR; }
