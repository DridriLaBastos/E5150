#include <stdio.h>
#include <malloc.h>

#include "../platform.h"

#include "Windows.h"

enum PLATFORM_CODE createFifo(const char* filePath)
{
	
}

//TODO: may benefit on some error checking
enum PLATFORM_CODE createProcess(const char* processName, const char** processCommandLineArgs, const size_t processCommandLineArgsCount)
{
	//For each arguments for the process we want to add a space inbetween, this creates processCommandLineArgsCount - 1 spaces.
	//We add 1 more to the size for the last null character
	//Then we add the size of each arguments of the process to create the final size of the process launch command line
	size_t processCommandLineSize = processCommandLineArgs;

	for (int i = 0; i < processCommandLineArgsCount; ++i)
	{
		processCommandLineSize += strlen(processCommandLineArgs[i]);
	}

	const size_t formatStringBufferSize = processCommandLineArgsCount * 3;

	//TODO: can the command line be large enougth that we need to use malloc here ?
	char* formatStringBuffer = _malloca(formatStringBufferSize);
	char* processLaunchCommandLine = _malloca(processCommandLineSize);

	memset(formatStringBuffer, 0, formatStringBufferSize);
	memset(processLaunchCommandLine, 0, processCommandLineSize);
	//Creation of the format string
	for (size_t i = 0; i < processCommandLineArgsCount; ++i)
	{
		strncpy(formatStringBuffer + i * 3, "%s ", 3);
	}

	snprintf_s(formatStringBuffer, processCommandLineSize, formatStringBuffer);

	//CreateProcess(NULL, processLaunchCommandLine, NULL, NULL, FALSE, 0, NULL, NULL);
}
