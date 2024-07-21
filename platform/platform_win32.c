#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "platform/platform.h"

#include <io.h>
#include <fcntl.h>
#include <Windows.h>
#include <namedpipeapi.h>

const int FIFO_OPEN_RDONLY = _O_RDONLY;
const int FIFO_OPEN_WRONLY = _O_WRONLY;

static const char* namedPipeSystemPath = "\\\\.\\pipe\\";

//+1 for the last NULL character
#define COMPUTE_WIN32_PIPE_PATH(pipeName)	const size_t pipeSystemPathLength = strlen(pipeName) + strlen(namedPipeSystemPath) + 1;\
											const char* pipeSystemPath = _malloca(pipeSystemPathLength);\
											strcpy_s(pipeSystemPath,pipeSystemPathLength,namedPipeSystemPath);\
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

static unsigned int mapInsert(const HANDLE handle, const char* id)
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

//The function will create the command line based on the values inside processArgs
//TODO: may benefit on some error checking
process_t platformCreateProcess(const char* processArgs[], const size_t processCommandLineArgsCount, FILE** childStdout, FILE** childStderr)
{
	//We want to add a space between each argument of the process, this creates processCommandLineArgsCount - 1 spaces.
	//We add 1 more to the size for the last null character
	//Then we add the size of each argument of the process to create the final size of the process launch command line
	size_t processCommandLineSize = processCommandLineArgsCount;

	for (int i = 0; i < processCommandLineArgsCount; ++i)
	{
		processCommandLineSize += strlen(processArgs[i]);
	}

    //TODO: Is it possible for this to return nullptr ?
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

	SECURITY_ATTRIBUTES pipeSecurityAttributes;
	pipeSecurityAttributes.bInheritHandle = TRUE;
	pipeSecurityAttributes.lpSecurityDescriptor = NULL;
	pipeSecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);

	HANDLE childStdoutRead;
	HANDLE childStdoutWrite;
	HANDLE childStderrRead;
	HANDLE childStderrWrite;

	BOOL result = CreatePipe(&childStdoutRead, &childStdoutWrite,&pipeSecurityAttributes,0);
	if (!result)
	{ return -1; }

	result = CreatePipe(&childStderrRead, &childStderrWrite, &pipeSecurityAttributes,0);
	if (!result)
	{ return -1; }

	//Child should not inherit the read handle of both stderr and stdout
	SetHandleInformation(&childStdoutRead, HANDLE_FLAG_INHERIT,0);
	SetHandleInformation(&childStderrRead, HANDLE_FLAG_INHERIT,0);

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));

	si.cb = sizeof(si);
	si.hStdOutput = childStdoutWrite;
	si.hStdError = childStderrWrite;
	si.dwFlags = STARTF_USESTDHANDLES;

	result = CreateProcess(NULL, processLaunchCommandLine, NULL, NULL, TRUE, 0, NULL, NULL,&si,&pi);

	//TODO: Why does WaitForSingleObject fails with timeout ?
	/*if (processCreationSucceed) {
		const DWORD waitStatus = WaitForSingleObject(pi.hProcess, INFINITE);

		processCreationSucceed = waitStatus == WAIT_OBJECT_0;
		
	}*/

	//Don't need write access to stdout and stderr
	CloseHandle(childStderrWrite);
	CloseHandle(childStdoutWrite);

	if (!result)
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return -1;
	}

	const int stdoutfd = _open_osfhandle((intptr_t)childStdoutRead,_O_RDONLY);
	const int stderrfd = _open_osfhandle((intptr_t)childStderrRead,_O_RDONLY);

	//rb are microsoft extension to open in binary mode. It breaks AINSI compliance but since we are on Win32 platform
	//code here, it is acceptable
	*childStdout = _fdopen(stdoutfd,"rb");
	*childStderr = _fdopen(stderrfd,"rb");

	return pi.dwProcessId;
}

enum PLATFORM_CODE platformTerminateProcess(const process_t process)
{
	BOOL b = GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, process);
	if (!b) { return PLATFORM_ERROR; }

	//TODO: Is it really necessary to call GetExitCode on window to clean the process ?
	HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, process);
	if (!h) { return PLATFORM_ERROR; }

	DWORD processExitCode;
	return GetExitCodeProcess(h, &processExitCode) ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

const char* platformError_GetDescription(void)
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

uint64_t platformError_GetCode(void) { return GetLastError(); }

static enum PLATFORM_CODE readFromChildHandle(char* const c, HANDLE h)
{
	DWORD numberOfBytesRead;
	BOOL result = ReadFile(h,c,sizeof(char),&numberOfBytesRead,NULL);

	if (result && (numberOfBytesRead == 0))
	{ return PLATFORM_STREAM_ENDS; }

	return result ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

enum PLATFORM_CODE platformFile_GetLastModificationTime(const char* filename, uint64_t* const datetime)
{
	HANDLE hfile = CreateFile(filename,0,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{ return PLATFORM_ERROR; }

	FILETIME lastAccessTime;
	if (!GetFileTime(hfile,NULL,&lastAccessTime,NULL))
	{
		return PLATFORM_ERROR;
	}

	const uint64_t fileDatetime = lastAccessTime.dwLowDateTime | ((uint64_t)lastAccessTime.dwHighDateTime << 32);
	*datetime = fileDatetime;
	return PLATFORM_SUCCESS;
}

enum PLATFORM_CODE platformFile_Copy(const char* from, const char* to)
{
	return CopyFile(from, to, FALSE) ? PLATFORM_SUCCESS : PLATFORM_SUCCESS;
}

static HMODULE hmodules[64];
static size_t hmoduleIndex = 0;

module_t platformDylib_Load(const char* const libpath)
{
	HMODULE hmodule = LoadLibrary(libpath);

	if (hmodule == NULL)
	{ return -1; }

	hmodules[hmoduleIndex] = hmodule;
	return hmoduleIndex++;
}

enum PLATFORM_CODE platformDylib_GetSymbolAddress(const module_t module, const char* const symbolname, void** address)
{
    HMODULE hmodule = hmodules[module];
	const FARPROC arproc = GetProcAddress(hmodule,symbolname);
	*address = arproc;
	return arproc ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

enum PLATFORM_CODE platformDylib_Release(const module_t module)
{
	if (module < 0) { return PLATFORM_SUCCESS; }
	hmoduleIndex -= 1;
	return FreeLibrary(hmodules[module]) ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}
