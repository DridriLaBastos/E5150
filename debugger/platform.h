#ifndef __PLATFORM_H__
#define __PLATFORM_H__

enum PLATFORM_CODE
{
	PLATFORM_FIFO_CREATION_SUCCESS = 0,
	PLATFORM_FIFO_CREATION_EXISTS,
	PLATFORM_ERROR
};

/**
* @brief Use platform code to create a fifo file for IPC
* 
* @param[in] name The name of fifo to be created
* 
* @return A positive value if the file have been created, a strictly negative value otherwise
* 
* @retval FIFO_CREATION_SUCCESS Fifo successfully created
* @retval FIFO_CREATION_EXISTS The fifo was already present and hasn't been created by this call
* @retval FIFO_CREATION_ERROR An error happened during creation
 */
enum PLATFORM_CODE createFifo(const char* filePath);
enum PLATFORM_CODE createProcess(const char* processName, const char** processCommandLineArgs, const size_t processCommandLineArgsCount);

#endif