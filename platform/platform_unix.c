#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/uio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "platform/platform.h"

static int stdoutfd[2];
static int stderrfd[2];
static bool dynamicLinkerError = false;

process_t platformCreateProcess(const char* processArgs[], const size_t processCommandLineArgsCount, FILE** childStdout, FILE** childStderr)
{
	pipe(stdoutfd);
	pipe(stderrfd);

	const pid_t createdPID = fork();

	if (createdPID == 0)
	{
		//We are in child process
		// dup2(stdoutfd[1], STDOUT_FILENO);
		// dup2(stderrfd[1], STDERR_FILENO);

		// close(stdoutfd[0]);   close(stdoutfd[1]);
		// close(stderrfd[0]);   close(stderrfd[1]);

		char** execFunctionArgs = alloca(sizeof(char*) * (processCommandLineArgsCount + 1));

		execFunctionArgs[processCommandLineArgsCount] = NULL;

		for (size_t i = 0; i < processCommandLineArgsCount; ++i)
		{ execFunctionArgs[i] = (char*)processArgs[i]; }

		//TODO: How to notify the emulator that the creation of the debugger failed ? Maybe using signal SIGUSR1/2?
		execvp(execFunctionArgs[0], (char *const *) execFunctionArgs);
		//If execvp returns -> an error happened
		perror("execvp");
		exit(EXIT_FAILURE);
	}
	close(stdoutfd[1]);
	close(stderrfd[1]);

	*childStdout = fdopen(stdoutfd[0],"r");
	*childStderr = fdopen(stderrfd[0],"r");

	setbuf(*childStdout, "L");
	setbuf(*childStderr, "L");

	return createdPID;
}

enum PLATFORM_CODE platformTerminateProcess(const process_t process)
{
	if (kill(process,SIGTERM) < 0)
	{
		//If the nice way doesn't work
		return kill(process,SIGKILL) != 0 ? PLATFORM_ERROR : PLATFORM_SUCCESS;
	}
	return waitpid(process,NULL,WUNTRACED) == process ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

enum PLATFORM_CODE platformCreateFifo (const char* fifoFileName)
{
	const int fifo = mkfifo(fifoFileName,S_IRWXG|S_IRWXO|S_IRWXU);

	if (fifo >= 0) { return PLATFORM_SUCCESS; }

	const int fifoAlreadyExists = errno == EEXIST;

	return fifoAlreadyExists ? PLATFORM_FIFO_ALREADY_CREATED : PLATFORM_ERROR;
}

int platformOpenFifo(const char* fifoFileName, const int openFlags)
{ return open(fifoFileName,openFlags); }

const char* platformGetLastErrorDescription(void)
{
	const char* errorstr = dynamicLinkerError ? dlerror() : strerror(errno);
	dynamicLinkerError = false;
	return errorstr;
}
uint64_t platformGetLastErrorCode(void)
{ return errno; }

static enum PLATFORM_CODE readFromChildFD(const int fd, char* const c)
{
	const ssize_t r = read(fd,c,1);

	if (r == sizeof(char))
		return PLATFORM_SUCCESS;
	return r == 0 ? PLATFORM_STREAM_ENDS : PLATFORM_ERROR;
}
enum PLATFORM_CODE platformReadChildSTDOUT(char* const c) { return readFromChildFD(stdoutfd[0], c); }
enum PLATFORM_CODE platformReadChildSTDERR(char* const c) { return readFromChildFD(stderrfd[0], c); }

enum PLATFORM_CODE platformFile_GetLastModificationTime(const char* filename, uint64_t* const datetime)
{
	struct stat filestat;
	if(stat(filename,&filestat))
	{ return PLATFORM_ERROR; }

	*datetime = filestat.st_atime;
	return PLATFORM_SUCCESS;
}

static void* modules [64];
static size_t moduleIndex = 0;

module_t platformDylib_Load(const char* const libpath)
{
	void* handle = dlopen(libpath,RTLD_LAZY);

	if (!handle)
	{
		dynamicLinkerError = true;
		return -1;
	}

	modules[moduleIndex] = handle;
	return moduleIndex++;
}

enum PLATFORM_CODE platformDylib_GetSymbolAddress(const module_t module, const char* const symbolname, void** address)
{
	void* handle = dlsym(modules[module],symbolname);

	//TODO: see man page, NULL return not always means error
	*address = handle;
	return handle ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

enum PLATFORM_CODE platformDylib_UpdateDylib(const module_t module, const char* const libpath)
{
	void* handle = modules[module];//Returns the same handle as in the first call

	if (dlclose(handle))
	{
		dynamicLinkerError = true;
		return PLATFORM_ERROR;
	}

	//TODO: what happens here when already loaded module points to the close dylib ?
	//		(nothing good I supposed)

	handle = dlopen(libpath,RTLD_LAZY);

	if (!handle)
	{
		dynamicLinkerError = true;
		return PLATFORM_ERROR;
	}

	modules[module] = handle;
	return PLATFORM_SUCCESS;
}

enum PLATFORM_CODE platformDylib_Release(const module_t module)
{
	if (module < 0) { return PLATFORM_SUCCESS; }
	moduleIndex -= 1;

	dynamicLinkerError = dlclose(modules[module]);

	return dynamicLinkerError ? PLATFORM_ERROR : PLATFORM_SUCCESS;
}
