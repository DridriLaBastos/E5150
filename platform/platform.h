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
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

enum PLATFORM_CODE
{
	PLATFORM_SUCCESS = 0,
	PLATFORM_ERROR,
	PLATFORM_FIFO_ALREADY_CREATED,
	PLATFORM_STREAM_ENDS
};

typedef int process_t;
typedef int module_t;

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
process_t platformCreateProcess(const char* processArgs[], const size_t processCommandLineArgsCount);
enum PLATFORM_CODE platformTerminateProcess(const process_t);

const char* platformGetLastErrorDescription(void);
const uint64_t platformGetLastErrorCode(void);

enum PLATFORM_CODE platformCreateFifo(const char* fifoFileName);
int platformOpenFifo(const char* fifoFileName, const int openFlags);

enum PLATFORM_CODE platformReadChildSTDOUT(char* const c);
enum PLATFORM_CODE platformReadChildSTDERR(char* const c);

enum PLATFORM_CODE platformFile_GetLastModificationTime(const char* filename, uint64_t* const datetime);

/**
 * Load the dynamic library specified at libpath
 * @param [in]libpath The path of the dybamic library to load
 * @retval -1 An error happened when loading the library
 * @return An ID referencing the loaded library on the platform dependent code
 */
module_t platformDylib_Load(const char* const libpath);

/**
 * Try to load a symbol from an already loaded dynamic library
 *
 * @param [in]module ID refere,cing the loaded library on the platform side
 * @param [in]symbolname The name of the symbol to laod
 * @param [in,out]address Pointer to the memory address containing the pointer to the code of the requested symbols.
 * On error referenced pointer is set to NULL
 * @retval PLATFORM_SUCCESS The symbol successfully loads
 * @retval PLATFORM_ERROR An error happened when loading the symbols. The code of the error can be retrieved by a
 * call to platformGetLastErrorCode and a description can be retrieved by a call to platformGetLastErrorDescription
 */
enum PLATFORM_CODE platformDylib_GetSymbolAddress(const module_t module, const char* const symbolname, void** address);
enum PLATFORM_CODE platformDylib_UpdateDylib(const module_t module, const char* const libpath);

#ifdef __cplusplus
}
#endif

#endif