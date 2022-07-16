#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stddef.h>
#include <stdint.h>

//Using string concatenation of the compilator to add strin delimiter to the path to handle spaced properly on windows
//TODO: test this : Why on my mac adding string delimiter didn't worked ?
#ifdef WIN32
#define PATH(path) "\"" path "\""
#else
#define PATH(path) path
#endif

enum PLATFORM_CODE
{
	PLATFORM_SUCCESS,
	PLATFORM_ERROR,
	PLATFORM_FIFO_ALREADY_CREATED
};

typedef int fifo_t;
typedef int process_t;

extern const int FIFO_OPEN_RDONLY;
extern const int FIFO_OPEN_WRONLY;

/**
 * @brief Creates a new process.
 * 
 *
 * The function calls the relevant platform code to create a new process. The function always returns into the parent process (unlike unix `fork` function). The return value identifies if an error occurs during the process creation or not.
 *
 * The way the process is created is by calling the first element in the processArgs array and passing the whole array
 * as the command line arguments of the created process. If the array values are ["python", "main.py], the function
 * will try to create the process `python`, and give it the argument `python main.py`.
 *
 * The function makes no assumption to the number of parameters a process can have.
 *
 * @param[in] processArgs cmd line arguments of the process (first is the process to be launch)
 * @param[in] processCommandLineArgsCount total number of paramters given to the process
 * @return -1 in case of failure or a value >= 0 that identifies the created process on the platform layer
 */
process_t processCreate(const char* processArgs [], const size_t processCommandLineArgsCount);
enum PLATFORM_CODE processTerminate(const process_t);

const char* errorGetDescription(void);
const uint64_t errorGetCode(void);

enum PLATFORM_CODE fifoCreate(const char* fifoFileName);
enum PLATFORM_CODE fifoClose(const fifo_t fifo);
enum PLATFORM_CODE fifoWrite(const fifo_t fifo, const void* data, const size_t noctet);
enum PLATFORM_CODE fifoRead(const fifo_t fifo, void* const buf, const size_t noctet);
fifo_t fifoOpen(const char* fifoFileName, const int openFlags);

#endif