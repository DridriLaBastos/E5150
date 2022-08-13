#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "debugger/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT void registerCommunicationFifos(const int fromDest, const int toDest);

DLL_EXPORT int writeToRegisteredDest(const void* const indata, const size_t size);
DLL_EXPORT int readFromRegisteredDest(void* const outdata, const size_t size);

void isEmulator(void);

#define READ_FROM_EMULATOR(ptr,size) readFromRegisteredDest(ptr,size)
#define WRITE_TO_EMULATOR(ptr,size) writeToRegisteredDest(ptr,size)

#define READ_FROM_DEBUGGER(ptr,size) readFromRegisteredDest(ptr,size)
#define WRITE_TO_DEBUGGER(ptr,size) writeToRegisteredDest(ptr,size)

#ifdef __cplusplus
}
#endif


#endif