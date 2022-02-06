#include <atomic>
#include <future>
#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "util.hpp"
#include "arch.hpp"
#include "debugger.hpp"

using namespace E5150;

static constexpr size_t USER_COMMAND_BUFFER_SIZE = 128;
static int debuggerSocketFd = -1;
static sockaddr_in debuggerAddr;
static FILE* debuggerFile = nullptr;
static std::atomic_bool debuggerOK = false;

static void debugerOKSignal(const int) { debuggerOK = true; }

/**
 * \brief An enum specifying how the debugger should react when a stop is needed
 */
enum class STOP_BEHAVIOUR
{
	STOP,///The debugger will stop if a stop is needed
	PASS ///The debugger will not stop if a stop is needed
};

/**
 * \brief An enum specifying how the debugger should react at each clock
 * */
enum class CLOCK_BEHAVIOUR
{
	PASS, ALWAYS_STOP, INSTRUCTION_STOP
};

enum PRINT_BEHAVIOUR
{
	PRINT_REGISTERS		= 1 << 0,
	PRINT_FLAGS			= 1 << 1,
	PRINT_INSTRUCTION	= 1 << 2,
	PRINT_CLOCK			= 1 << 3,
	PRINT_ALL			= ~0
};

struct DebuggerState
{
	STOP_BEHAVIOUR  stopBehaviour;
	CLOCK_BEHAVIOUR clockBehaviour;
	PRINT_BEHAVIOUR printBehaviour;
};

static DebuggerState state;

void E5150::Debugger::init()
{
	state.clockBehaviour = CLOCK_BEHAVIOUR::ALWAYS_STOP;
	state.stopBehaviour  = STOP_BEHAVIOUR::STOP;

	static int debuggerPipes [2];

	if (pipe(debuggerPipes) < 0)
	{
		WARNING("Unable to create communication pipes with the debugger. [ERRNO]: '{}'", strerror(errno));
		return;
	}

	const pid_t debuggerPID = fork();

	if (debuggerPID < 0)
	{
		WARNING("Unable to create the debugger subprocess. [ERRNO]: '{}'", strerror(errno));
		return;
	}

	if (debuggerPID != 0)
	{
		INFO("Debugger process created with pid {}", debuggerPID);
		close(debuggerPipes[0]);
		debuggerFile = fdopen(debuggerPipes[1],"w");
	}
	else
	{
		close(debuggerPipes[1]);
		if (execlp("python3", "/usr/bin/python3", "/Users/adrien/Documents/Informatique/C++/E5150/core/debugger/debugger.py", NULL) < 0)
		{
			WARNING("Unable to launch the debugger script. [ERRNO]: {}", strerror(errno));
			exit(127);
		}
	}

	// debuggerFile = popen("python3 /Users/adrien/Documents/Informatique/C++/E5150/core/debugger/debugger.py", "w");

	// if (!debuggerFile)
	// {
	// 	WARNING("Unable to create a communcication channel to the debugger. [ERRNO]: '{}'", strerror(errno));
	// 	return;
	// }

	const std::string emulatorPIDForDebugger = std::to_string(getpid()).append("\n");
	printf("'%s'\n",emulatorPIDForDebugger.data());
	fputs(emulatorPIDForDebugger.data(), debuggerFile);

	signal(SIGUSR1,debugerOKSignal);

	// debuggerSocketFd = socket(AF_INET, SOCK_STREAM,0);

	// if (debuggerSocketFd == -1)
	// {
	// 	WARNING("Cannot create connection to debugger. errno error: '{}'", strerror(errno));
	// 	return;
	// }

	// debuggerAddr.sin_family	= AF_INET;
	// debuggerAddr.sin_port	= htons(5510);

	// if (inet_aton("127.0.0.1",&debuggerAddr.sin_addr) == 0)
	// {
	// 	WARNING("Cannot create debugger connection address data. errno message: '{}'", strerror(errno));
	// 	return;
	// }

	// if (connect(debuggerSocketFd, (const sockaddr*)&debuggerAddr, sizeof(debuggerAddr)) != 0)
	// {
	// 	WARNING("Cannot connect to the debugger. errno message: '{}'", strerror(errno));
	// 	return;
	// }

	// INFO("Connected succesfully to the debugger");
}

void E5150::Debugger::deinit()
{
	if (debuggerFile)
		pclose(debuggerFile);
}

static void printRegisters(void)
{
	printf("CS: %#6.4x   DS: %#6.4x   ES: %#6.4x   SS: %#6.4x\n",cpu.cs,cpu.ds,cpu.es,cpu.ss);
	printf("AX: %#6.4x   bx: %#6.4x   CX: %#6.4x   DX: %#6.4x\n",cpu.ax,cpu.bx,cpu.cx,cpu.dx);
	printf("SI: %#6.4x   DI: %#6.4x   BP: %#6.4x   SP: %#6.4x\n\n",cpu.si,cpu.di,cpu.bp,cpu.sp);
}

static void printFlags(void)
{
	std::cout << ((cpu.flags & CPU::CARRY) ? "CF" : "cf") << "  " << ((cpu.flags & CPU::PARRITY) ? "PF" : "pf") << "  " << ((cpu.flags & CPU::A_CARRY) ? "AF" : "af") << "  " << ((cpu.flags & CPU::ZERRO) ? "ZF" : "zf") << "  " << ((cpu.flags & CPU::SIGN) ? "SF" : "sf") << "  " << ((cpu.flags & CPU::TRAP) ? "TF" : "tf") << "  " << ((cpu.flags & CPU::INTF) ? "IF" : "if") << "  " << ((cpu.flags & CPU::DIR) ? "DF" : "df") << "  " << ((cpu.flags & CPU::OVER) ? "OF" : "of") << std::endl;
}

static void conditionnalyPrintInstruction(void)
{
	if (state.printBehaviour & PRINT_BEHAVIOUR::PRINT_REGISTERS)
		printRegisters();
				
	if (state.printBehaviour & PRINT_BEHAVIOUR::PRINT_FLAGS)
		printFlags();
}

static void printBIUDebugInfo(void)
{
	const I8086::BIU::InternalInfos& BIUDebugInfo = I8086::BIU::getDebugWorkingState();

	switch (BIUDebugInfo.workingMode)
	{
		case I8086::BIU::WORKING_MODE::FETCH_INSTRUCTION:
		{
			printf("BIU: BUS CYCLE %d (clock count down: %d) --- FETCHING %#5x (%#4x:%#4x)\n", BIUDebugInfo.BUS_CYCLE_CLOCK - BIUDebugInfo.BUS_CYCLE_CLOCK_LEFT, BIUDebugInfo.BUS_CYCLE_CLOCK_LEFT,cpu.genAddress(cpu.cs,cpu.ip+BIUDebugInfo.IP_OFFSET),cpu.cs,cpu.ip+BIUDebugInfo.IP_OFFSET);

			if ((cpu.biu.instructionBufferQueuePos <= 5) && BIUDebugInfo.BUS_CYCLE_CLOCK_LEFT == BIUDebugInfo.BUS_CYCLE_CLOCK)
			{
				printf("BIU: INSTRUCTION BUFFER QUEUE: queue size %d\n", cpu.biu.instructionBufferQueuePos);

				printf("Instruction buffer: ");
				std::for_each(cpu.biu.instructionBufferQueue.begin(), cpu.biu.instructionBufferQueue.end(),
					[](const uint8_t b) { printf("%#x ",b); });
				// for (uint8_t b: cpu.biu.instructionBufferQueue)
				// 	printf("%#x ",b);
				putchar('\n');
			}
		} break;

		case I8086::BIU::WORKING_MODE::FETCH_DATA:
			printf("BIU: DATA ACCESS FROM EU: clock left: %d\n", BIUDebugInfo.EU_DATA_ACCESS_CLOCK_LEFT);
			break;
		
		case I8086::BIU::WORKING_MODE::WAIT_ROOM_IN_QUEUE:
			printf("BIU: INSTRUCTION BUFFER QUEUE FULL\n");
			break;

		case I8086::BIU::WORKING_MODE::WAIT_END_OF_INTERRUPT_DATA_SAVE_SEQUENCE:
			printf("BIU: WAITING END OF INTERRUPT DATA SAVING PROCEDURE\n");
			break;

	default:
		break;
	}

}

static void printCurrentInstruction(const I8086::EU::InternalInfos& workingState)
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	if (!inst)
		return;
	std::cout << std::hex << workingState.CURRENT_INSTRUCTION_CS << ":" << workingState.CURRENT_INSTRUCTION_IP << " (" << cpu.genAddress(workingState.CURRENT_INSTRUCTION_CS,workingState.CURRENT_INSTRUCTION_IP) << ")" << std::dec << ": ";
	std::cout << xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(&cpu.eu.decodedInst)) << " : length = " << xed_decoded_inst_get_length(&cpu.eu.decodedInst) << std::endl;
	std::cout << xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(&cpu.eu.decodedInst)) << " ";
	unsigned int realOperandPos = 0;
	bool foundPtr = false;

	for (unsigned int i = 0; i < xed_decoded_inst_noperands(&cpu.eu.decodedInst); ++i)
	{
		const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, i));
		const xed_operand_visibility_enum_t op_vis = xed_operand_operand_visibility(xed_inst_operand(inst, i));

		if (true/*op_vis == XED_OPVIS_EXPLICIT*/)
		{
			if (foundPtr)
			{
				std::cout << ":";
				foundPtr = false;
			}
			else
			{
				if (realOperandPos > 0)
					std::cout<< ", ";
			}

			switch (op_name)
			{
			case XED_OPERAND_RELBR:
				std::cout << (xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst) & 0xFFFF);
				break;

			case XED_OPERAND_PTR:
				std::cout << std::hex << (xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst) & 0xFFFF) << std::dec;
				foundPtr = true;
				break;

			case XED_OPERAND_REG0:
			case XED_OPERAND_REG1:
			case XED_OPERAND_REG2:
				std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
				break;

			case XED_OPERAND_IMM0:
			case XED_OPERAND_IMM1:
				std::cout << std::hex << (xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst) & 0xFFFF) << std::dec;
				break;

			//Displaying memory operand with format SEG:[[BASE +] [INDEX +] DISPLACEMENT ]
			case XED_OPERAND_MEM0:
			{
				const xed_reg_enum_t baseReg = xed_decoded_inst_get_base_reg(&cpu.eu.decodedInst, 0);
				const xed_reg_enum_t indexReg = xed_decoded_inst_get_index_reg(&cpu.eu.decodedInst, 0);
				const int64_t memDisplacement = xed_decoded_inst_get_memory_displacement(&cpu.eu.decodedInst,0);
				std::cout << ((xed_decoded_inst_get_memory_operand_length(&cpu.eu.decodedInst, 0) == 1) ? "BYTE" : "WORD") << " ";
				std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_seg_reg(&cpu.eu.decodedInst, 0)) << ":[";

				if (baseReg != XED_REG_INVALID)
					std::cout << xed_reg_enum_t2str(baseReg);
				
				if (indexReg != XED_REG_INVALID)
				{
					if (baseReg != XED_REG_INVALID)
						std::cout << " + ";
					std::cout << xed_reg_enum_t2str(indexReg);
				}

				if ((indexReg != XED_REG_INVALID) || (baseReg != XED_REG_INVALID))
				{
					if (memDisplacement != 0)
					{
						if (memDisplacement > 0)
							std::cout << " + " << memDisplacement;
						else
							std::cout << " - " << -memDisplacement;
					}
				}
				else
					std::cout << memDisplacement;
				std::cout<< "]";
			}	break;

			default:
				break;
			}

			++realOperandPos;
		}
	}
		std::cout << " (iform: ' "<< xed_iform_enum_t2str(xed_decoded_inst_get_iform_enum(&cpu.eu.decodedInst)) << "')";
		std::cout << " (" << cpu.instructionExecutedCount+1 << ")\n";
}

static void printEUDebugInfo(void)
{
	const I8086::EU::InternalInfos& EUDebugInfo = I8086::EU::getDebugWorkingState();

	switch (EUDebugInfo.EUWorkingMode)
	{
		case I8086::EU::WORKING_MODE::EXEC_INSTRUCTION:
			printCurrentInstruction(EUDebugInfo);
			printf("Clock cycle: %d\n", EUDebugInfo.INSTRUCTION_CLOCK_LEFT);
			break;

		default:
			break;
	}
}

static void printDebugInfo (const bool instructionExecuted)
{
	switch (state.clockBehaviour)
	{
		case CLOCK_BEHAVIOUR::PASS:
			break;
		
		case CLOCK_BEHAVIOUR::ALWAYS_STOP:
		{
			printBIUDebugInfo();
			printEUDebugInfo();
			// cpu.biu.debugClockPrint();
			//cpu.eu.debugClockPrint();
		}

		case CLOCK_BEHAVIOUR::INSTRUCTION_STOP:
		{
			if (instructionExecuted)
				conditionnalyPrintInstruction();
		} break;
	}
}

static int parseCommand(const std::string& userInput)
{
	const bool step = (userInput == "s") || (userInput == "step") || (userInput.empty());
	if (step)
	{
		state.clockBehaviour = CLOCK_BEHAVIOUR::ALWAYS_STOP;
		return true;
	}

	const bool istep = (userInput == "i") || (userInput == "istep");
	if (istep)
	{
		state.clockBehaviour = CLOCK_BEHAVIOUR::INSTRUCTION_STOP;
		return true;
	}

	const bool cont = (userInput == "c") || (userInput == "continue");
	if (cont)
	{
		state.clockBehaviour = CLOCK_BEHAVIOUR::PASS;
		E5150::Util::CURRENT_DEBUG_LEVEL = 0;
		return true;
	}

	const bool quit = (userInput == "q") || (userInput == "quit");
	if (quit)
	{
		state.stopBehaviour = STOP_BEHAVIOUR::PASS;
		E5150::Util::_continue = false;
		return true;
	}

	const bool help = (userInput == "h") || (userInput == "help");
	if (help)
	{
		puts("This is the debugger cli for E5150. You can type a command to perform what you want");
		puts("This screen while show a bref description of available commands");
		puts(" 'c', 'continue' continue the emulation without stopping until a special condition is raised");
		puts(" 's', 'step'     continue the emulation stopping at each clock");
		puts(" 'i', 'istep'    continue the emulation stopping at each instructions");
		puts(" 'h', 'help'     display this text and wait for another command");
		puts(" 'q', 'quit'     ends the emulation");
		puts("");
		return false;
	}

	WARNING("Command not recognized. Type 'c' or 'continue' to continue. Type 'h' or 'help' to display available commands");
	return false;
}

static void debuggerCLI(void)
{
	if (state.stopBehaviour == STOP_BEHAVIOUR::PASS)
		return;

	static std::string userInput;

	do
	{
		std::cout << " > ";
		std::getline(std::cin,userInput);
		std::transform(userInput.begin(), userInput.end(), userInput.begin(), ::tolower);
		/* code */
	} while (!parseCommand(userInput));
}

static void printDebuggerCLI(const bool instructionExecuted)
{
	switch (state.clockBehaviour)
	{
		case CLOCK_BEHAVIOUR::PASS:
			break;

		case CLOCK_BEHAVIOUR::INSTRUCTION_STOP:
			if (!instructionExecuted) { break; }
	
		default://CLOCK_BEHAVIOUR::ALWAYS_STOP
			debuggerCLI();
			break;
	}
}

void Debugger::wakeUp(const uint8_t instructionExecuted)
{
	// printDebugInfo(instructionExecuted);
	// printDebuggerCLI(instructionExecuted);
	// static uint8_t debugServerAck;

	// /**
	//  * 1 - 1 octet of data is sent to wake up the debugger
	//  * 2 - 1 octet of data is excpected from the debugger to be synchronized with it
	//  * */
	// const ssize_t sent = send(debuggerSocketFd,(void*)(&instructionExecuted),sizeof(uint8_t),0);
	// const ssize_t received = recv(debuggerSocketFd,(void*)&debugServerAck, sizeof(debugServerAck), MSG_WAITALL);
	// printf("send: %d   received: %d\n", sent, received);
	
	static std::string debuggerCmd;
	while(!debuggerOK)
	{
		std::getline(std::cin, debuggerCmd);
		//getline deletes the last \n so it is added to the string sent to stdin of the debugger so that the debuger knows the command is ended
		debuggerCmd.append("\n");
		std::cout << "'" << debuggerCmd << "'\n";
		fputs(debuggerCmd.data(), debuggerFile);
	}
	debuggerOK = false;
}