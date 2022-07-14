#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stddef.h>
#include <stdint.h>

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

enum PLATFORM_CODE processCreate(const char* processArgs [], const size_t processCommandLineArgsCount);
enum PLATFORM_CODE processWait(const process_t process);
enum PLATFORM_CODE processKill(const process_t process);
int platformGetProcessID(const process_t process);

enum PLATFORM_CODE fifoCreate(const char* fifoFileName);
const char* platformGetErrorDescription(void);
fifo_t fifoOpen(const char* fifoFileName, const int openFlags);

enum PLATFORM_CODE fifoClose(const fifo_t fifo);
enum PLATFORM_CODE fifoRemove(const char* fifoFilename);
enum PLATFORM_CODE fifoWrite(const fifo_t fifo, const void* data, const size_t noctet);
enum PLATFORM_CODE fifoRead(const fifo_t fifo, void* const buf, const size_t noctet);

#endif