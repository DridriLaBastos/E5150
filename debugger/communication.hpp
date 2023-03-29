//
// Created by Adrien COURNAND on 26/03/2023.
//

#ifndef E5150_COMMUNICATION_HPP
#define E5150_COMMUNICATION_HPP

namespace E5150::DEBUGGER {
	enum class STREAM {
		STDOUT, STDERR
	};
	constexpr unsigned int COMMUNICATION_TEST_VALUE = 0xDEAB12CD;
	void setDebuggerServerStreamFilePtr(const STREAM streamType ,FILE* const newStreamPtr);
	FILE* getDebuggerServerStreamFilePtr(const STREAM streamType);
}

#endif //E5150_COMMUNICATION_HPP
