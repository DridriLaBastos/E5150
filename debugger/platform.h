#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stddef.h>
#include <stdint.h>

#ifdef WIN32
#include <fcntl.h>
#else
#include <unistd.h>
#endif

//Using string concatenation feature of the compiler to add string delimiter to the path to handle spaced properly on windows
//TODO: test this : Why on my mac adding string delimiter didn't worked ?
#ifdef WIN32
#define PATH(path) "\"" path "\""
#else
#define PATH(path) path
#endif

#ifdef WIN32
#define FIFO_PATH(path) "//./pipe/" path
#else
#define FIFO_PATH(path) path
#endif

#ifdef WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

enum PLATFORM_CODE
{
	PLATFORM_SUCCESS,
	PLATFORM_ERROR,
	PLATFORM_FIFO_ALREADY_CREATED
};

typedef int process_t;

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Creates a new process.
	 *
	 *
	 * Calls the relevant platform code to create a new process. The function always returns into the parent process
	 * (unlike unix `fork` function). The return value identifies if an error occurred during the process creation or not.
	 *
	 * The first element of the processArgs array will be interpreted as the command line to run and passed.
	 * The whole array will also be passed as the argument to the program. Thus the cmd line to run the program is also
	 * the first argument of the program. If the array values are ["python", "main.py"], the function
	 * will try to create the process `python`, and give it the argument `python main.py`.
	 *
	 * @param[in] processArgs cmd line arguments of the process (first is the process to be launch)
	 * @param[in] processCommandLineArgsCount total number of paramters given to the process
	 * @return -1 in case of failure or a value >= 0 that identifies the created process on the platform layer
	 */
	process_t processCreate(const char* processArgs[], const size_t processCommandLineArgsCount);
	enum PLATFORM_CODE processTerminate(const process_t);

	const char* errorGetDescription(void);
	const uint64_t errorGetCode(void);

	enum PLATFORM_CODE fifoCreate(const char* fifoFileName);
	int fifoOpen(const char* fifoFileName, const int openFlags);

#ifdef __cplusplus
}
#endif

#endif