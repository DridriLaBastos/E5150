#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdio.h>
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

process_t platformCreateProcess(const char* processArgs[], const size_t processCommandLineArgsCount, FILE** childStdout, FILE** childStderr);
enum PLATFORM_CODE platformTerminateProcess(const process_t process);

const char* platformGetLastErrorDescription(void);
uint64_t platformGetLastErrorCode(void);

enum PLATFORM_CODE platformCreateFifo(const char* fifoFileName);
int platformOpenFifo(const char* fifoFileName, const int openFlags);

enum PLATFORM_CODE platformFile_GetLastModificationTime(const char* filename, uint64_t* const datetime);
enum PLATFORM_CODE platformFile_Copy(const char* from, const char* to);

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

/**
 * Release a previously loaded library.
 *
 * @param [in]module ID referencing the library to be unloaded on the platform side
 * @retval PLATFORM_SUCCESS Library successfully unloaded
 * @retval PLATFORM_ERROR An error happened on unloading
 */
enum PLATFORM_CODE platformDylib_Release(const module_t module);

#ifdef __cplusplus
}
#endif

#endif