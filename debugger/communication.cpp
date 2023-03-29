//
// Created by Adrien COURNAND on 29/03/2023.
//

#include <stdio.h>

#include "communication.hpp"

static FILE* debuggerStdoutStream = nullptr;
static FILE* debuggerStderrStream = nullptr;

void E5150::DEBUGGER::setDebuggerServerStreamFilePtr(const E5150::DEBUGGER::STREAM streamType,
                                                     FILE *const newStreamPtr)
{
	FILE** targetStream = &(streamType == STREAM::STDOUT ? debuggerStdoutStream : debuggerStderrStream);
	*targetStream = newStreamPtr;
 }

 FILE* E5150::DEBUGGER::getDebuggerServerStreamFilePtr(const E5150::DEBUGGER::STREAM streamType)
 {
	return (streamType == STREAM::STDOUT ? debuggerStdoutStream : debuggerStderrStream);
 }
