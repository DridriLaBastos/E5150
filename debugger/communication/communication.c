
//TODO: cleaner file : can this could go inside platform.h
#ifndef WIN32
#include <fcntl.h>
#else
#include <corecrt_io.h>
#endif

#include "communication.h"

static int fromEmulator = -1;
static int toEmulator = -1;

void registerCommunicationFifos (const int _fromEmulator, const int _toEmulator)
{
	fromEmulator = _fromEmulator;
	toEmulator = _toEmulator;
}

//TODO: To be safer : send the amount of data that will be sent and of the mirroring functions [read,write]FromDebugger
void writeToEmulator(const uint8_t* const indata, const size_t size){ write(toEmulator,indata,size); }
void readFromEmulator(uint8_t* const outdata, const size_t size) { read(fromEmulator,outdata,size); }
