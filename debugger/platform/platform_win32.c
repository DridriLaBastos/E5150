#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "../platform.h"

#include <io.h>
#include <fcntl.h>
#include <Windows.h>
#include <namedpipeapi.h>

const int FIFO_OPEN_RDONLY = _O_RDONLY;
const int FIFO_OPEN_WRONLY = _O_WRONLY;

static const char* namedPipeSystemPath = "\\\\.\\pipe\\";

static HANDLE childStdoutRead;
static HANDLE childStdoutWrite;
static HANDLE childStderrRead;
static HANDLE childStderrWrite;

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

//The function will create the command line based on the values inside processArgs
//TODO: may benefit on some error checking
process_t platformCreateProcess(const char* processArgs[], const size_t processCommandLineArgsCount)
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

	if (!result)
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(childStderrWrite);
		CloseHandle(childStdoutWrite);
		return -1;
	}

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

const char* platformGetLastErrorDescription(void)
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

const uint64_t platformGetLastErrorCode() { return GetLastError(); }

enum PLATFORM_CODE platformCreateFifo(const char* fifoFileName)
{
	COMPUTE_WIN32_PIPE_PATH(fifoFileName);
	//TODO: Not sure about that, seems to work.
	//What is the difference between MESSAGE_MODE and BYTE_MODE ? My (little) understanding is that with MESSAGE_MODE
    // the pipe tries to read/write the n number bytes we give. With BYTE_MODE it's up to me to verify that I get the
    // number of bytes I asked to send or receive.
    //
	//What is the difference between PIPE_TYPE_MESSAGE with PIPE_READMODE_MESSAGE and PIPE_TYPE_BYTE with PIPE_READ_MODE message ?
	//
	//What I did for now is a byte pipe with READMODE_MESSAGE : it's still byte but we want to read/write the whole
    // number of bytes we asked... ? Is it good ? Bad ? Okay but stupid ?
	//I have no idea...
    //Update some weeks later : it seems to work fine with only integers to read/write
	const HANDLE namedPipeHandle = CreateNamedPipe(pipeSystemPath, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 256, 256, 0, NULL);

	if (namedPipeHandle == INVALID_HANDLE_VALUE) { return PLATFORM_ERROR; }

	mapInsert(namedPipeHandle, fifoFileName);
	return PLATFORM_SUCCESS;
}

int platformOpenFifo(const char* fifoFileName, const int openFlags)
{
	const int mapEntryIndex = mapFindIndex(fifoFileName);
	if (mapEntryIndex < 0) { return -1; }

	const HANDLE hFifo = map[mapEntryIndex].handle;
	const BOOL b = ConnectNamedPipe(hFifo, NULL);

	return _open_osfhandle(hFifo, openFlags);
}

static enum PLATFORM_CODE readFromChildHandle(char* const c, HANDLE h)
{
	DWORD numberOfBytesRead;
	BOOL result = ReadFile(h,c,sizeof(char),&numberOfBytesRead,NULL);

	if (result && (numberOfBytesRead == 0))
	{ return PLATFORM_STREAM_ENDS; }

	return result ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}

//TODO: How to detect end of file ?
enum PLATFORM_CODE platformReadChildSTDOUT(char* const c) { return readFromChildHandle(c,childStdoutRead); }
enum PLATFORM_CODE platformReadChildSTDERR(char* const c) { return readFromChildHandle(c,childStderrRead); }

enum PLATFORM_CODE platformFile_GetLastModificationTime(const char* filename, uint64_t* const datetime)
{
	HANDLE hfile = CreateFile(filename,GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,NULL);

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

static HMODULE hmodules[64];
static size_t hmoduleIndex = 0;

module_t platformDylib_Load(const char* const libpath)
{
	HMODULE hmodule = LoadLibrary(libpath);

	if (hmodule == NULL)
	{ return -1; }

	hmodules[hmoduleIndex] = hmodules;
	return hmoduleIndex++;
}

enum PLATFORM_CODE platformDylib_GetSymbolAddress(const module_t module, const char* const symbolname, void** address)
{
	const FARPROC arproc = GetProcAddress(hmodules[module],TEXT(symbolname));
	*address = arproc;
	return arproc ? PLATFORM_SUCCESS : PLATFORM_ERROR;
}
