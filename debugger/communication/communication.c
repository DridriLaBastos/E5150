
//TODO: cleaner file : can this could go inside platform.h
#ifdef WIN32
#include <corecrt_io.h>
#else
#include <fcntl.h>
#endif

//#define DEBUG_DE_COM

#ifdef DEBUG_DE_COM
#include <stdio.h>
#endif

#include "communication.h"

static int fromDest = -1;
static int toDest = -1;
static size_t contextCharIndex = 0;
static const char context [] = {'D', 'E'};

void registerCommunicationFifos (const int _fromDest, const int _toDest)
{
	fromDest = _fromDest;
	toDest = _toDest;
}

void isEmulator(void) { contextCharIndex = 1; }

int writeToRegisteredDest(const void* const indata, const size_t size)
{
#ifdef DEBUG_DE_COM
	static unsigned int called = 0;
	called += 1;
	printf("%c WRITE %d: %zu bytes", context[contextCharIndex], called, size);
	for (size_t i = 0; i < size; ++i)
	{ printf(" %#x",((uint8_t*)indata)[i]); }
	putchar('\n');
#endif
	return write(toDest,indata,size);
}

int readFromRegisteredDest(void* const outdata, const size_t size)
{
#ifdef DEBUG_DE_COM
	static unsigned int called = 0;
	const int readStatus = read(fromDest,outdata,size);
	called += 1;
	printf("%c READ %d: %zu bytes", context[contextCharIndex], called, size);
	for (size_t i = 0; i < size; ++i)
	{ printf(" %#x",((uint8_t*)outdata)[i]); }
	putchar('\n');
	return readStatus;
#else
	return read(fromDest,outdata,size);
#endif
}

