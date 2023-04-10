//
// Created by Adrien COURNAND on 26/03/2023.
//
#include <fstream>
#include <unistd.h>

#include "debugger/communication.h"

char cwd[PATH_MAX];

int main(int argc, const char* argv[]) {
	printf("[E5150 DEBUGGER SERVER] Debugger server running from '%s'\n", getcwd(cwd,PATH_MAX));

	decom_InitCommunication(DECOM_CONFIGURE_DEBUGGER);

	unsigned int testValue;
	decom_TestConnection(DECOM_CONFIGURE_DEBUGGER,&testValue);
	printf("received 0x%X\n",testValue);

	if (testValue != COMMUNICATION_TEST_VALUE)
	{
		fprintf(stderr, "[E5150 DEBUGGER SERVER] Expected to receive 0x%x but got 0x%x. Communication between emulator and debugger may not work properly"
			,COMMUNICATION_TEST_VALUE,testValue);
	}

#if 0
	argparse::ArgumentParser program ("debugger",DEBUGGER_VERSION);

	program.add_description("Embedded debugger for the E5150 emulator");

	program.add_argument("--output_fifo_path", "-o")
	.help("Path to a communication channel that respects a file API from the debugger to the emulator")
	.required();

	program.add_argument("--input_fifo_path","-i")
	.help("Path to a communication channel that respects a file API from the emulator to the debugger")
	.required();

	try {
		program.parse_args(argc,argv);
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	const auto inputFifoPath = program.get<std::string>("-i");
	const auto outputFifoPath = program.get<std::string>("-o");

	std::ifstream fromEmulator(inputFifoPath,std::ios_base::in | std::ios_base::binary);
	std::ofstream toEmulator (outputFifoPath,std::ios_base::out | std::ios_base::binary);

	//Unbuffered mode for all buffers
	fromEmulator.rdbuf()->pubsetbuf(nullptr,0);
	toEmulator.rdbuf()->pubsetbuf(nullptr,0);

	if (!fromEmulator.is_open())
	{
		fprintf(stderr, "Unable to open fifo path '%s'\n",inputFifoPath.c_str());
		return EXIT_FAILURE;
	}

	if (!toEmulator.is_open())
	{
		fprintf(stderr, "Unable to open fifo path '%s'\n",outputFifoPath.c_str());
		return EXIT_FAILURE;
	}


	//Unbuffered mode for all buffers
	unsigned int connectionTestValue;
	fromEmulator.read((char*)&connectionTestValue,sizeof(connectionTestValue));

	if (connectionTestValue != E5150::DEBUGGER::COMMUNICATION_TEST_VALUE) {
		fprintf(stderr, "Error while trying to read protocole test value from the debugger.\n"
						"\tExpected value was 0x%x got 0x%x\n",E5150::DEBUGGER::COMMUNICATION_TEST_VALUE, connectionTestValue);
		return EXIT_FAILURE;
	}

	fprintf(stderr,"[E5150 DEBUGGER] Connected to the emulator");
#endif
	return EXIT_SUCCESS;
}