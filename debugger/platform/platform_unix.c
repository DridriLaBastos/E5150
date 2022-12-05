#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "../platform.h"

static int stdoutfd[2];
static int stderrfd[2];

process_t processCreate(const char* processCommandLineArgs[], const size_t processCommandLineArgsCount)
{
	pipe(stdoutfd);
	pipe(stderrfd);

	const pid_t createdPID = fork();

	if (createdPID == 0)
	{
		dup2(stdoutfd[1], STDOUT_FILENO);
		dup2(stderrfd[1], STDERR_FILENO);

		close(stdoutfd[0]);   close(stdoutfd[1]);
		close(stderrfd[0]);   close(stderrfd[1]);

		//We are in child process
		char** execFunctionArgs = alloca(sizeof(char*) * (processCommandLineArgsCount + 1));

		execFunctionArgs[processCommandLineArgsCount] = NULL;

		for (int i = 0; i < processCommandLineArgsCount; ++i)
		{ execFunctionArgs[i] = processCommandLineArgs[i]; }

		//TODO: How to notify the emulator that the creation of the debugger failed ? Maybe using signal SIGUSR1/2?
		exit(execvp(execFunctionArgs[0], execFunctionArgs));
	}
	close(stdoutfd[1]);
	close(stderrfd[1]);

	return createdPID;
}

enum PLATFORM_CODE processTerminate(process_t process)
{
	if (kill(process,SIGTERM) < 0)
	{
		//If the nice way doesn't work
		return kill(process,SIGKILL) != 0 ? PLATFORM_ERROR : PLATFORM_SUCCESS;
	}
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

static enum PLATFORM_CODE readFromChildFD(const int fd, char* const c)
{
	const ssize_t r = read(fd,c,1);

	if (r == sizeof(char))
		return PLATFORM_SUCCESS;
	return r == 0 ? PLATFORM_STREAM_ENDS : PLATFORM_ERROR;
}
enum PLATFORM_CODE readChildStdout(char* const c) { return readFromChildFD(stdoutfd[0],c); }
enum PLATFORM_CODE readChildStderr(char* const c) { return readFromChildFD(stderrfd[0],c); }
