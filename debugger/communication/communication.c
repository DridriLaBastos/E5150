#include "communication.h"
#include "platform.h"

static fifo_t fromEmulator = -1;
static fifo_t toEmulator = -1;

void registerCommunicationFifos (const int _fromEmulator, const int _toEmulator)
{
	fromEmulator = _fromEmulator;
	toEmulator = _toEmulator;
}

void writeToEmulator(const uint8_t* const indata, const size_t size){ fifoWrite(toEmulator,indata,size); }
void readFromEmulator(uint8_t* const outdata, const size_t size) { fifoRead(fromEmulator,outdata,size); }
