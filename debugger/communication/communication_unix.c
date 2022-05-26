#include <stdio.h>
#include <unistd.h>

#include "communication.h"


static int fromEmulator = -1;
static int toEmulator = -1;

void registerCommunicationFifos (const int _fromEmulator, const int _toEmulator)
{
	fromEmulator = _fromEmulator;
	toEmulator = _toEmulator;
}

void writeToEmulator(const uint8_t* const indata, const size_t size){ printf("DE WRITE\n"); write(toEmulator,indata,size); }
void readFromEmulator(uint8_t* const outdata, const size_t size) { printf("DE READ\n"); read(fromEmulator,outdata,size); }