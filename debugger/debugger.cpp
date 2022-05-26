#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include "util.hpp"
#include "arch.hpp"
#include "debugger.hpp"
#include "communication/command.h"
#include "debuggerCommandExecStatus.hpp"

using namespace E5150;

constexpr char EMULATOR_TO_DEBUGGER_FIFO_FILENAME[] = ".ed.fifo";
constexpr char DEBUGGER_TO_EMULATOR_FIFO_FILENAME[] = ".de.fifo";

static int toDebugger = -1;
static int fromDebugger = -1;
static pid_t debuggerPID = -1;

static struct {
	COMMAND_TYPE commandType;
	unsigned int commandSubtype;
	unsigned int targetValue;
	unsigned int currentValue;

	void clear (void)
	{
		commandType = COMMAND_TYPE_ERROR;
		commandSubtype = 0;
		targetValue = 0;
		currentValue = 0;
	}
} commandExecutionContext;

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
	unsigned int printBehaviour;
};

static DebuggerState state;

void E5150::Debugger::init()
{
	state.clockBehaviour = CLOCK_BEHAVIOUR::ALWAYS_STOP;
	state.stopBehaviour  = STOP_BEHAVIOUR::STOP;
	commandExecutionContext.commandType = COMMAND_TYPE_ERROR;

	if (mkfifo(EMULATOR_TO_DEBUGGER_FIFO_FILENAME, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
	{
		if (errno != EEXIST)
		{
			WARNING("Cannot initiate send channel communication with the debugger. [ERRNO]: '{}'", strerror(errno));
			return;
		}

		INFO("Read channel file exists. This usually means that the program was not properly closed previously");
	}

	if (mkfifo(DEBUGGER_TO_EMULATOR_FIFO_FILENAME, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
	{
		if (errno != EEXIST)
		{
			WARNING("Cannot initiate receive channel communication with the debugger. [ERRNO]: '{}'", strerror(errno));
			return;
		}

		INFO("Write channel file exists. This usually means that the program was not properly closed previously");
	}

	const pid_t emulatorPID = getpid();
	debuggerPID = fork();

	if (debuggerPID < 0)
	{
		WARNING("Unable to create the debugger subprocess. [ERRNO]: '{}'", strerror(errno));
		return;
	}

	if (debuggerPID != 0)
	{
		INFO("Debugger process created with pid {}", debuggerPID);
		toDebugger = open(EMULATOR_TO_DEBUGGER_FIFO_FILENAME, O_WRONLY);
		if (toDebugger < 0)
		{
			INFO("Unable to open send to debugger channel. [ERRNO]: '{}'", strerror(errno));
			deinit();
			return;
		}

		fromDebugger = open(DEBUGGER_TO_EMULATOR_FIFO_FILENAME, O_RDONLY);
		if (fromDebugger < 0)
		{
			INFO("Unable to open receive from debugger channel. [ERRNO]: '{}'", strerror(errno));
			deinit();
			return;
		}

		const uint8_t sizeofPID_t = sizeof(pid_t);
		write(toDebugger, &sizeofPID_t,1);
		write(toDebugger, &emulatorPID,sizeofPID_t);
	}
	else
	{
		if (execlp("python3", "/usr/bin/python3", "/Users/adrien/Documents/Informatique/C++/E5150/debugger/debugger.py", EMULATOR_TO_DEBUGGER_FIFO_FILENAME, DEBUGGER_TO_EMULATOR_FIFO_FILENAME, NULL) < 0)
		{
			WARNING("Unable to launch the debugger script. [ERRNO]: {}", strerror(errno));
			exit(127);
		}
	}
}

void E5150::Debugger::deinit()
{
	close(toDebugger);
	close(fromDebugger);

	remove(EMULATOR_TO_DEBUGGER_FIFO_FILENAME);
	remove(DEBUGGER_TO_EMULATOR_FIFO_FILENAME);

	kill(debuggerPID, SIGKILL);
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

static void conditionnalyPrintInstruction(void)
{
	if (state.printBehaviour & PRINT_BEHAVIOUR::PRINT_REGISTERS)
		printRegisters();
				
	if (state.printBehaviour & PRINT_BEHAVIOUR::PRINT_FLAGS)
		printFlags();
}

static void handleContinueCommand()
{
	static CONTINUE_TYPE continueType;
	static unsigned int continueValue;

	read(fromDebugger, &continueType, sizeof(CONTINUE_TYPE));
	read(fromDebugger, &continueValue, sizeof(unsigned int));

	if (continueType == CONTINUE_TYPE_BUS) { continueValue *= 4; }

	INFO("continue command type: {}   value {}", continueType, continueValue);
	commandExecutionContext.commandType = COMMAND_TYPE_CONTINUE;
	commandExecutionContext.commandSubtype = (unsigned int)continueType;
	commandExecutionContext.targetValue = continueValue;
	commandExecutionContext.currentValue = 0;
}

static void handleDisplayCommand()
{
	static int displayFlags[4];

	//read(fromDebugger,&displayFlags[0], sizeof(int));
	// read(fromDebugger,&displayFlags[1], sizeof(int));
	// read(fromDebugger,&displayFlags[2], sizeof(int));
	// read(fromDebugger,&displayFlags[3], sizeof(int));

	// if (displayFlags[0]) { state.printBehaviour ^= PRINT_BEHAVIOUR::PRINT_FLAGS; }
	// if (displayFlags[1]) { state.printBehaviour ^= PRINT_BEHAVIOUR::PRINT_INSTRUCTION; }
	// if (displayFlags[2]) { state.printBehaviour ^= PRINT_BEHAVIOUR::PRINT_REGISTERS; }
	// if (displayFlags[3] > -1)
	// {
	// 	if (displayFlags[3] > DEBUG_LEVEL_MAX)
	// 	{
	// 		INFO("Log level too high set to highest value {}", DEBUG_LEVEL_MAX);
	// 		displayFlags[3] = DEBUG_LEVEL_MAX;
	// 	}
	// 	E5150::Util::CURRENT_DEBUG_LEVEL = displayFlags[3];
	// }

	INFO("print behaviour (log level={}){}{}{}",
		E5150::Util::CURRENT_DEBUG_LEVEL,
		state.printBehaviour & PRINT_BEHAVIOUR::PRINT_FLAGS ? " print flags" : "",
		state.printBehaviour & PRINT_BEHAVIOUR::PRINT_INSTRUCTION ? " print instructions" : "",
		state.printBehaviour & PRINT_BEHAVIOUR::PRINT_REGISTERS ? " print registers" : "");
}

void Debugger::wakeUp(const uint8_t instructionExecuted)
{	
	static COMMAND_TYPE commandType;
	static uint8_t shouldStop;
	static uint64_t instructionExecutedToSend = 0;
	instructionExecutedToSend += instructionExecuted;

	if (commandExecutionContext.commandType != COMMAND_TYPE_ERROR)
	{
		switch(commandExecutionContext.commandType)
		{
			case COMMAND_TYPE_CONTINUE:
			{
				commandExecutionContext.currentValue += commandExecutionContext.commandSubtype == CONTINUE_TYPE_INSTRUCTION ? instructionExecuted : 1;
			} break;
		}
		if (commandExecutionContext.currentValue < commandExecutionContext.targetValue)
		{ return; }
	}

	commandExecutionContext.clear();

	conditionnalyPrintInstruction();
	write(toDebugger,&instructionExecutedToSend,8);
	instructionExecutedToSend = 0;

	do
	{
		read(fromDebugger, &commandType,sizeof(COMMAND_TYPE));
		printf("REACHED WITH %d\n",commandType);
		const COMMAND_RECEIVED_STATUS commandReceivedStatus = commandType >= COMMAND_TYPE_ERROR ? COMMAND_RECEIVED_FAILURE : COMMAND_RECEIVED_SUCCESS;

		write(toDebugger, &commandReceivedStatus, sizeof(commandReceivedStatus));

		switch (commandType)
		{
			case COMMAND_TYPE_CONTINUE:
				handleContinueCommand();
				break;
			
			case COMMAND_TYPE_STEP:
				INFO("Get command step");
				break;
			
			case COMMAND_TYPE_DISPLAY:
				handleDisplayCommand();
				break;

			default:
				WARNING("Unknown response from debugger, behaviour will be unpredicatable");
				break;
		}
		read(fromDebugger,&shouldStop,1);
	} while (!shouldStop);
}
