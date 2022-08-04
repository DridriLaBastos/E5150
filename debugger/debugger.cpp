#include "util.hpp"
#include "arch.hpp"
#include "debugger.hpp"
#include "communication/command.h"

#include <cerrno>
#include <cstring>

#include "platform.h"

using namespace E5150;

#define EMULATOR_TO_DEBUGGER_FIFO_FILENAME ".ed.fifo"
#define DEBUGGER_TO_EMULATOR_FIFO_FILENAME ".de.fifo"

static int toDebugger = -1;
static int fromDebugger = -1;
static process_t debuggerProcess = -1;
static unsigned int savedLogLevel = 0;
static bool debuggerInitialized = false;

static struct
{
	COMMAND_TYPE type;
	unsigned int subtype;
	union
	{
		unsigned int value;
		unsigned int count;
	};
	
	void clear(void)
	{
		type = COMMAND_TYPE_ERROR;
		subtype = 0;
		value = 0;
	}

} context;

void E5150::Debugger::init()
{
	context.clear();

	if (const PLATFORM_CODE code = fifoCreate(EMULATOR_TO_DEBUGGER_FIFO_FILENAME); code != PLATFORM_SUCCESS)
	{
		if (code == PLATFORM_ERROR)
		{
			E5150_WARNING("Cannot initiate send channel communication with the debugger. Emulation will continue without the debugger. [PLATFORM ERROR {}]: '{}'", errorGetCode(), errorGetDescription());
			return;
		}

		E5150_INFO("Read channel file exists. This usually means that the program was not properly closed previously");
	}

	if (const PLATFORM_CODE code = fifoCreate(DEBUGGER_TO_EMULATOR_FIFO_FILENAME); code != PLATFORM_SUCCESS)
	{
		if (code == PLATFORM_ERROR)
		{
			E5150_WARNING("Cannot initiate send channel communication with the debugger.  Emulation will continue without the debugger. [PLATFORM ERROR {}]: '{}'", errorGetCode(), errorGetDescription());
			return;
		}

		E5150_INFO("Read channel file exists. This usually means that the program was not properly closed previously");
	}

	const char* debuggerArgs [] = {
		PATH(PYTHON3_EXECUTABLE_PATH),
		PATH(DEBUGGER_PYTHON_SCRIPT_PATH),
		EMULATOR_TO_DEBUGGER_FIFO_FILENAME,
		DEBUGGER_TO_EMULATOR_FIFO_FILENAME,
		PATH(DECOM_LIB_PATH)
	};

	debuggerProcess = processCreate(debuggerArgs,std::size(debuggerArgs));

	if (debuggerProcess == -1)
	{
		E5150_WARNING("Unable to create the debugger subprocess. Emulation will continue without the debugger. [PLATFORM ERROR {}] '{}'.", errorGetCode(), errorGetDescription());
		return;
	}

	toDebugger = fifoOpen(EMULATOR_TO_DEBUGGER_FIFO_FILENAME, O_WRONLY);
	if (toDebugger < 0)
	{
		E5150_WARNING("Unable to open send to debugger channel. Emulation will continue without the debugger. [ERRNO {}]: '{}'", errno, strerror(errno));
		deinit();
		return;
	}

	fromDebugger = fifoOpen(DEBUGGER_TO_EMULATOR_FIFO_FILENAME, O_RDONLY);
	if (fromDebugger < 0)
	{
		E5150_WARNING("Unable to open receive from debugger channel. Emulation will continue without the debugger. [ERRNO {}]: '{}'", errno, strerror(errno));
		deinit();
		return;
	}

	constexpr uint32_t debuggerSynchronizationData = 0xDEAB12CD;
	if (write(toDebugger,&debuggerSynchronizationData,sizeof(debuggerSynchronizationData)) < 0)
	{
		E5150_WARNING("Unable to send data to the debugger.  Emulation will continue without the debugger. [ERRNO {}]: '{}'", errno, strerror(errno));
		deinit();
		return;
	}

	debuggerInitialized = true;
}

void E5150::Debugger::deinit()
{
	if (processTerminate(debuggerProcess) != PLATFORM_SUCCESS)
	{ E5150_INFO("Cannot stop debugger process. [PLATFORM ERROR {}]: {}", errorGetCode(), errorGetDescription()); }
	
	if (close(toDebugger)   == -1) { E5150_DEBUG("Error when closing " EMULATOR_TO_DEBUGGER_FIFO_FILENAME ". [ERRNO {}] {}", errno, strerror(errno)); }
	if (close(fromDebugger) == -1) { E5150_DEBUG("Error when closing " DEBUGGER_TO_EMULATOR_FIFO_FILENAME ". [ERRNO {}] {}", errno, strerror(errno)); }

	remove(FIFO_PATH(EMULATOR_TO_DEBUGGER_FIFO_FILENAME));
	remove(FIFO_PATH(DEBUGGER_TO_EMULATOR_FIFO_FILENAME));
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

static void printCurrentInstruction(void)
{
	const I8086::EU::InternalInfos& workingState = cpu.eu.getDebugWorkingState();
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

		if (op_vis == XED_OPVIS_EXPLICIT)
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
		std::cout << " (" << cpu.instructionExecutedCount << ")\n";
}

static void printInstruction(void)
{
		printRegisters();
		printFlags();
		printCurrentInstruction();
}

static void handleContinueCommand()
{
	context.type = COMMAND_TYPE_CONTINUE;
	read(fromDebugger, &context.subtype, sizeof(CONTINUE_TYPE));
	read(fromDebugger, &context.count, sizeof(unsigned int));

	savedLogLevel = E5150::Util::CURRENT_EMULATION_LOG_LEVEL;
	E5150::Util::CURRENT_EMULATION_LOG_LEVEL = 0;
}

static void handleStepCommand()
{
	uint8_t stepFlags;
	read(fromDebugger,&stepFlags,1);

	if ((stepFlags & STEP_TYPE_CLOCK) && (stepFlags & STEP_TYPE_PASS))
	{ E5150_INFO("Using pass flag with clock has no effects"); }

	context.type = COMMAND_TYPE_STEP;
	context.subtype = stepFlags;
	context.count = 1;
}

static void handleDisplayCommand()
{
	int newLogLevel;
	read(fromDebugger, &newLogLevel, sizeof(newLogLevel));

	if (newLogLevel < 0)
	{
		E5150_INFO("Current log level : {}", E5150::Util::CURRENT_EMULATION_LOG_LEVEL);
		return;
	}

	if (newLogLevel > EMULATION_MAX_LOG_LEVEL)
	{
		E5150_INFO("Log level select to high, applied log level will be the highest value : {}", EMULATION_MAX_LOG_LEVEL);
		E5150::Util::CURRENT_EMULATION_LOG_LEVEL = EMULATION_MAX_LOG_LEVEL;
		return;
	}

	E5150_INFO("Log level set to new level {}", newLogLevel);
	E5150::Util::CURRENT_EMULATION_LOG_LEVEL = newLogLevel;
}

static void handleQuitCommand(void)
{
	E5150::Util::_continue = false;
}


static bool isControlTransferIn( const xed_iclass_enum_t& iclass )
{
	return	(iclass == XED_ICLASS_CALL_FAR) || (iclass == XED_ICLASS_CALL_NEAR) ||
			(iclass == XED_ICLASS_JMP) || (iclass == XED_ICLASS_JMP_FAR) ||
			(iclass == XED_ICLASS_JZ)|| (iclass == XED_ICLASS_JL)|| (iclass == XED_ICLASS_JLE)|| (iclass == XED_ICLASS_JB)|| (iclass == XED_ICLASS_JBE)|| (iclass == XED_ICLASS_JP)|| (iclass == XED_ICLASS_JO)|| (iclass == XED_ICLASS_JS)|| (iclass == XED_ICLASS_JNZ)|| (iclass == XED_ICLASS_JNL)|| (iclass == XED_ICLASS_JNLE)|| (iclass == XED_ICLASS_JNB)|| (iclass == XED_ICLASS_JNBE)|| (iclass == XED_ICLASS_JNP)|| (iclass == XED_ICLASS_JNS)|| (iclass == XED_ICLASS_JCXZ) ||
			(iclass == XED_ICLASS_INT) || (iclass == XED_ICLASS_INT1) || (iclass == XED_ICLASS_INT3) || (iclass == XED_ICLASS_INTO);
}

static bool isControlTranferOut(const xed_iclass_enum_t& iclass)
{ return (iclass == XED_ICLASS_RET_FAR) || (iclass == XED_ICLASS_RET_NEAR) || (iclass == XED_ICLASS_IRET); }

FORCE_INLINE static bool executeUntilNextInstruction(const bool instructionExecuted, const bool instructionDecoded)
{
	context.count -= instructionExecuted && (context.count > 0);
	return (context.count > 0) || (!instructionDecoded);
}

static bool executeContinueCommand(const bool instructionExecuted, const bool instructionDecoded)
{
	if (context.subtype == CONTINUE_TYPE_INFINITE) { return true; }

	switch (context.subtype)
	{
		case CONTINUE_TYPE_CLOCK:
			context.count -= 1;
			return context.count;
		
		case CONTINUE_TYPE_INSTRUCTION:
			return executeUntilNextInstruction(instructionExecuted, instructionDecoded);
	}

	return false;//To silent compiler warning
}

static void printClockLevelBIUEmulationLog(void)
{
	const auto& BIUState = cpu.biu.getDebugWorkingState();

	switch (BIUState.workingMode)
	{
	case I8086::BIU::WORKING_MODE::FETCH_INSTRUCTION:
	{
		EMULATION_INFO_LOG<EMULATION_MAX_LOG_LEVEL>("BIU: BUS CYCLE {} (clock count down: {}) --- FETCHING {} ({}:{})", BIUState.BUS_CYCLE_CLOCK - BIUState.BUS_CYCLE_CLOCK_LEFT, BIUState.BUS_CYCLE_CLOCK_LEFT, cpu.genAddress(cpu.cs, cpu.ip + BIUState.IP_OFFSET), cpu.cs, cpu.ip + BIUState.IP_OFFSET);

			//TODO: This should be given inside the state variable
		if ((cpu.biu.instructionBufferQueuePos <= 5) && BIUState.BUS_CYCLE_CLOCK_LEFT == BIUState.BUS_CYCLE_CLOCK)
		{
			EMULATION_INFO_LOG<EMULATION_MAX_LOG_LEVEL>("BIU: INSTRUCTION BUFFER QUEUE: queue size {}", cpu.biu.instructionBufferQueuePos);

			//TODO: Investigate how to use spdlog to produce an equivalent output
			if (E5150::Util::CURRENT_EMULATION_LOG_LEVEL >= EMULATION_MAX_LOG_LEVEL)
			{
				printf("Instruction buffer: ");
				std::for_each(cpu.biu.instructionBufferQueue.begin(), cpu.biu.instructionBufferQueue.end(),
					[](const uint8_t b) { printf("%#x ", b); });
				// for (uint8_t b: cpu.biu.instructionBufferQueue)
				// 	printf("%#x ",b);
				putchar('\n');
			}
		}
	} break;

	case I8086::BIU::WORKING_MODE::FETCH_DATA:
		EMULATION_INFO_LOG<EMULATION_MAX_LOG_LEVEL>("BIU: DATA ACCESS FROM EU: clock left: {}", BIUState.EU_DATA_ACCESS_CLOCK_LEFT);
		break;

	case I8086::BIU::WORKING_MODE::WAIT_ROOM_IN_QUEUE:
		EMULATION_INFO_LOG < EMULATION_MAX_LOG_LEVEL>("BIU: INSTRUCTION BUFFER QUEUE FULL");
		break;

	case I8086::BIU::WORKING_MODE::WAIT_END_OF_INTERRUPT_DATA_SAVE_SEQUENCE:
		EMULATION_INFO_LOG < EMULATION_MAX_LOG_LEVEL>("BIU: WAITING END OF INTERRUPT DATA SAVING PROCEDURE");
		break;

	default:
		break;
	}
}

static void printClockLevelEUEmulationLog(void)
{
	const auto& EUInfo = cpu.eu.getDebugWorkingState();
	
}

static bool executeStepCommand(const bool instructionExecuted, const bool instructionDecoded)
{
	printClockLevelBIUEmulationLog();
	printClockLevelEUEmulationLog();
	if (context.subtype & STEP_TYPE_CLOCK) { return false; }
	if (context.subtype & STEP_TYPE_INSTRUCTION) { return executeUntilNextInstruction(instructionExecuted,instructionDecoded); }
	//TODO: STEP_TYPE_INSTRUCTION & STEP_TYPE_PASS
	return false;
}


//TODO: better design pattern for command execution
static bool executeCommand(const uint8_t instructionExecuted, const bool instructionDecoded)
{
	if (context.type == COMMAND_TYPE_ERROR) { return false; }

	switch (context.type)
	{
		case COMMAND_TYPE_CONTINUE:
			return executeContinueCommand(instructionExecuted,instructionDecoded);
		
		case COMMAND_TYPE_STEP:
			return executeStepCommand(instructionExecuted,instructionDecoded);
		
		default:
			return false;
	}

	// 	case (COMMAND_TYPE_STEP):
	// 	{
	// 		printBIUDebugInfo();
	// 		printEUDebugInfo();
	// 		if (context.subtype & STEP_TYPE_INSTRUCTION)
	// 		{
	// 			if (instructionDecoded & (context.subtype & STEP_TYPE_PASS))
	// 			{
	// 				const xed_iclass_enum_t iclass = xed_decoded_inst_get_iclass(&cpu.eu.decodedInst);
	// 				context.count += isControlTransferIn(iclass);
	// 				context.count -= isControlTranferOut(iclass);
	// 			}
	// 			else
	// 			{ context.count -= instructionExecuted; }
	// 		}
	// 		else
	// 		{ context.count -= 1; }
	// 	}
	// }
}

//TODO: Debugger error resilient : if sending a data to the debugger failed, stop using it and continue the emulation as if they were no debugger
void Debugger::wakeUp(const uint8_t instructionExecuted, const bool instructionDecoded)
{	
	static COMMAND_TYPE commandType;
	static uint8_t shouldStop;
	static const uint8_t commandEndSynchro = 0;//Only here to notify the debugger the emulator finished executing the command
	static bool unizializedDebuggerWarningNotPrinted = true;

	if (!debuggerInitialized) {
		if (unizializedDebuggerWarningNotPrinted) {
			E5150_WARNING("Debugger was not successfully initialized. Debugger features will not be available.");
			unizializedDebuggerWarningNotPrinted = false;
		}
		return;
	}
	if (executeCommand(instructionExecuted, instructionDecoded)) { return; }
	
	E5150::Util::CURRENT_EMULATION_LOG_LEVEL = savedLogLevel;
	context.clear();
	printInstruction();

	if(write(toDebugger,&cpu.instructionExecutedCount,8) == -1) { return; }
	
	do
	{
		if (read(fromDebugger, &commandType,sizeof(COMMAND_TYPE)) == -1) { break; }
		const COMMAND_RECEIVED_STATUS commandReceivedStatus = commandType >= COMMAND_TYPE_ERROR ? COMMAND_RECEIVED_FAILURE : COMMAND_RECEIVED_SUCCESS;

		if(write(toDebugger, &commandReceivedStatus, sizeof(commandReceivedStatus)) == -1) { break; }

		switch (commandType)
		{
			case COMMAND_TYPE_CONTINUE:
				handleContinueCommand();
				break;
			
			case COMMAND_TYPE_STEP:
				handleStepCommand();
				break;
			
			case COMMAND_TYPE_DISPLAY:
				handleDisplayCommand();
				break;

			case COMMAND_TYPE_QUIT:
				handleQuitCommand();
				break;

			default:
				E5150_WARNING("Unknown response from debugger, behaviour may be unpredicatable");
				break;
		}
		if (read(fromDebugger,&shouldStop,1) < 0) { break; }
		if (write(toDebugger, &commandEndSynchro, 1) < 0) { break;; }
	} while (!shouldStop);
}
