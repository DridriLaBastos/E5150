#include "core/util.hpp"
#include "core/arch.hpp"
#include "debugger.hpp"
#include "communication/command.h"
#include "communication/communication.h"

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
static bool debuggerInitialized = true;

static struct
{
	COMMAND_TYPE type;

	union {
		unsigned int passType;
		unsigned int subtype;
	};

	union
	{
		unsigned int value;
		unsigned int count;
	};
	
	void clear(void)
	{
		type = COMMAND_TYPE_ERROR;
		passType = 0;
		subtype = 0;
		value = 0;
	}

} context;

void E5150::Debugger::init()
{
	context.clear();
	isEmulator();

	if (const PLATFORM_CODE code = fifoCreate(EMULATOR_TO_DEBUGGER_FIFO_FILENAME); code != PLATFORM_SUCCESS)
	{
		if (code == PLATFORM_ERROR)
		{
			E5150_WARNING("Cannot initiate send channel communication with the debugger. Emulation will continue without the debugger. [PLATFORM ERROR {}]: '{}'", errorGetCode(), errorGetDescription());
			return;
		}

		E5150_INFO("Read channel file exists. This usually means that the program was not properly closed previously");
		E5150_WARNING("\tIf this behaviour persists, remove the file '" EMULATOR_TO_DEBUGGER_FIFO_FILENAME "'");
	}

	if (const PLATFORM_CODE code = fifoCreate(DEBUGGER_TO_EMULATOR_FIFO_FILENAME); code != PLATFORM_SUCCESS)
	{
		if (code == PLATFORM_ERROR)
		{
			E5150_WARNING("Cannot initiate send channel communication with the debugger.  Emulation will continue without the debugger. [PLATFORM ERROR {}]: '{}'", errorGetCode(), errorGetDescription());
			return;
		}

		E5150_INFO("Read channel file exists. This usually means that the program was not properly closed previously");
		E5150_WARNING("\tIf this behaviour persists, remove the file '" DEBUGGER_TO_EMULATOR_FIFO_FILENAME "'");
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

	registerCommunicationFifos(fromDebugger, toDebugger);

	constexpr uint32_t debuggerSynchronizationData = 0xDEAB12CD;
	if (WRITE_TO_DEBUGGER(&debuggerSynchronizationData, sizeof(debuggerSynchronizationData)) < 0)
	{
		E5150_WARNING("Unable to send data to the debugger.  Emulation will continue without the debugger. [ERRNO {}]: '{}'", errno, strerror(errno));
		deinit();
		return;
	}
}

void E5150::Debugger::deinit()
{
    if (!debuggerInitialized)
        return;

	if (debuggerProcess >= 0)
	{
		if (processTerminate(debuggerProcess) != PLATFORM_SUCCESS)
		{ E5150_INFO("Cannot stop debugger process. [PLATFORM ERROR {}]: {}", errorGetCode(), errorGetDescription()); }
		else { debuggerProcess = -1; }
	}

	if (toDebugger >= 0)
	{
		if (close(toDebugger)   == -1) { E5150_DEBUG("Error when closing " EMULATOR_TO_DEBUGGER_FIFO_FILENAME ". [ERRNO {}] {}", errno, strerror(errno)); }
		else { toDebugger = -1; }
	}

	if (fromDebugger >= 0)
	{
		if (close(fromDebugger) == -1) { E5150_DEBUG("Error when closing " DEBUGGER_TO_EMULATOR_FIFO_FILENAME ". [ERRNO {}] {}", errno, strerror(errno)); }
		else { fromDebugger = -1; }
	}

	remove(FIFO_PATH(EMULATOR_TO_DEBUGGER_FIFO_FILENAME));
	remove(FIFO_PATH(DEBUGGER_TO_EMULATOR_FIFO_FILENAME));

    debuggerInitialized = false;
}

static void printRegisters(void)
{
	printf("CS: %#6.4x   DS: %#6.4x   ES: %#6.4x   SS: %#6.4x\n",cpu.regs.cs,cpu.regs.ds,cpu.regs.es,cpu.regs.ss);
	printf("AX: %#6.4x   bx: %#6.4x   CX: %#6.4x   DX: %#6.4x\n",cpu.regs.ax,cpu.regs.bx,cpu.regs.cx,cpu.regs.dx);
	printf("SI: %#6.4x   DI: %#6.4x   BP: %#6.4x   SP: %#6.4x\n\n",cpu.regs.si,cpu.regs.di,cpu.regs.bp,cpu.regs.sp);
}

static void printFlags(void)
{
	std::cout << ((cpu.regs.flags & CPU::CARRY) ? "CF" : "cf") << "  " << ((cpu.regs.flags & CPU::PARRITY) ? "PF" : "pf") << "  " << ((cpu.regs.flags & CPU::A_CARRY) ? "AF" : "af") << "  " << ((cpu.regs.flags & CPU::ZERRO) ? "ZF" : "zf") << "  " << ((cpu.regs.flags & CPU::SIGN) ? "SF" : "sf") << "  " << ((cpu.regs.flags & CPU::TRAP) ? "TF" : "tf") << "  " << ((cpu.regs.flags & CPU::INTF) ? "IF" : "if") << "  " << ((cpu.regs.flags & CPU::DIR) ? "DF" : "df") << "  " << ((cpu.regs.flags & CPU::OVER) ? "OF" : "of") << std::endl;
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

static void printCpuInfos(void)
{
		printRegisters();
		printFlags();
		printCurrentInstruction();
}

static void handleContinueCommand()
{
	context.type = COMMAND_TYPE_CONTINUE;
	READ_FROM_DEBUGGER(&context.passType, sizeof(PASS_TYPE));
	READ_FROM_DEBUGGER(&context.count, sizeof(unsigned int));

	savedLogLevel = E5150::Util::CURRENT_EMULATION_LOG_LEVEL;
	E5150::Util::CURRENT_EMULATION_LOG_LEVEL = 0;
}

static void handleStepCommand()
{
	uint8_t passFlags;
	READ_FROM_DEBUGGER(&passFlags,1);

	if ((passFlags & PASS_TYPE_CLOCKS) && (passFlags & PASS_TYPE_STEP_THROUGH))
	{
		E5150_INFO("Using pass flag with clock has no effects");
		passFlags &= ~PASS_TYPE_STEP_THROUGH;
	}

	if ((passFlags & PASS_TYPE_INSTRUCTIONS) && (passFlags & PASS_TYPE_STEP_THROUGH))
	{ passFlags = PASS_TYPE_STEP_THROUGH; }

	context.type = COMMAND_TYPE_STEP;
	context.passType = passFlags;
	context.count = 1;
}

static void handleDisplayCommand()
{
	int newLogLevel;
	READ_FROM_DEBUGGER(&newLogLevel, sizeof(newLogLevel));

	if (newLogLevel < 0)
	{
		E5150_INFO("Current log level : {}", E5150::Util::CURRENT_EMULATION_LOG_LEVEL);
		return;
	}

	if (newLogLevel > EMULATION_MAX_LOG_LEVEL)
	{
		E5150_INFO("Log level selected to high, log level applied will be the highest value : {}", EMULATION_MAX_LOG_LEVEL);
		E5150::Util::CURRENT_EMULATION_LOG_LEVEL = EMULATION_MAX_LOG_LEVEL;
		return;
	}

	E5150_INFO("Log level set to the new level {}", newLogLevel);
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

static void printClockLevelBIUEmulationLog(void)
{
	const auto& BIUState = cpu.biu.getDebugWorkingState();

	switch (BIUState.workingMode)
	{
	case I8086::BIU::WORKING_MODE::FETCH_INSTRUCTION:
	{
		EMULATION_INFO_LOG<EMULATION_MAX_LOG_LEVEL>("BIU: BUS CYCLE {} (clock count down: {}) --- FETCHING 0x{:X} (0x{:X}:0x{:X})", BIUState.BUS_CYCLE_CLOCK - BIUState.BUS_CYCLE_CLOCK_LEFT, BIUState.BUS_CYCLE_CLOCK_LEFT, cpu.genAddress(cpu.regs.cs, cpu.regs.ip + BIUState.IP_OFFSET), cpu.regs.cs, cpu.regs.ip + BIUState.IP_OFFSET);

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
		EMULATION_INFO_LOG<EMULATION_MAX_LOG_LEVEL>("BIU: INSTRUCTION BUFFER QUEUE FULL");
		break;

	case I8086::BIU::WORKING_MODE::WAIT_END_OF_INTERRUPT_DATA_SAVE_SEQUENCE:
		EMULATION_INFO_LOG<EMULATION_MAX_LOG_LEVEL>("BIU: WAITING END OF INTERRUPT DATA SAVING PROCEDURE");
		break;

	default:
		break;
	}
}

static void printClockLevelEUEmulationLog(void)
{
	const auto& EUInfo = cpu.eu.getDebugWorkingState();
}

/**
 * @brief Executes the continue and step commands
 * 
 * @param instructionExecuted the CPU finishes to execute an instruction
 * @param instructionDecoded the CPU has decoded an instruction
 * @retval true the command is still being executed
 * @retval false the command execution is done
 */
static bool executePassCommand(const uint8_t instructionExecuted, const bool instructionDecoded)
{
	switch (context.passType)
	{
		case PASS_TYPE_CLOCKS:
			printClockLevelBIUEmulationLog();
			printClockLevelEUEmulationLog();
			context.count -= 1;
			return context.count;

		case PASS_TYPE_INSTRUCTIONS:
			//When the execution of a command is done, the emulator will stop and display the state of the cpu.
			//If at the end of the execution, a new instruction hasn't been decoded, the CPU will still display that the current
			//instruction executed is the previous one, and it can be confusing for the user. Instead, when count is 0, the command
			//still maintain its execution state until a new instruction have been decoded.
			context.count -= (instructionExecuted) && (context.count > 0);
			return (context.count > 0) || (!instructionDecoded);
		//case STEP_TROUGH
		case PASS_TYPE_INFINITE:
			return true;

		default:
			E5150_WARNING("Unknown way execute a pass command (continue/step). Command aborted");
			break;
	}
	return false;
}

//TODO: IMPORTANT: Debugger error resilient : if sending a data to the debugger failed, stop using it and continue the emulation as if they were no debugger
void Debugger::wakeUp(const uint8_t instructionExecuted, const bool instructionDecoded)
{	
	COMMAND_TYPE commandType;
	uint8_t shouldStop;
	const uint8_t commandEndSynchro = 0;//Only here to notify to the debugger that the emulator finished executing the command
	bool unizializedDebuggerWarningNotPrinted = true;

	if (!debuggerInitialized) {
		if (unizializedDebuggerWarningNotPrinted) {
			E5150_WARNING("Debugger was not successfully initialized. Debugger features will not be available.");
			unizializedDebuggerWarningNotPrinted = false;
		}
		return;
	}

	if (context.type == COMMAND_TYPE_CONTINUE || context.type == COMMAND_TYPE_STEP)
	{
		if (executePassCommand(instructionExecuted, instructionDecoded)) { return; }
		if (context.type == COMMAND_TYPE_CONTINUE) { E5150::Util::CURRENT_EMULATION_LOG_LEVEL = savedLogLevel; }
	}
	
	context.clear();
	printCpuInfos();
	if(WRITE_TO_DEBUGGER(&cpu.instructionExecutedCount,8) == -1) { return; }
	
	do
	{
		if (READ_FROM_DEBUGGER(&commandType,sizeof(COMMAND_TYPE)) == -1) { break; }
		const COMMAND_RECEIVED_STATUS commandReceivedStatus = commandType >= COMMAND_TYPE_ERROR ? COMMAND_RECEIVED_FAILURE : COMMAND_RECEIVED_SUCCESS;

		if(WRITE_TO_DEBUGGER( &commandReceivedStatus, sizeof(commandReceivedStatus)) == -1) { break; }

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
				E5150_WARNING("Unknown response from debugger, behaviour may be unpredictable");
				break;
		}
		if (READ_FROM_DEBUGGER(&shouldStop,1) < 0) { break; }
		if (WRITE_TO_DEBUGGER(&commandEndSynchro, 1) < 0) { break; }

		shouldStop |= context.type == COMMAND_TYPE_QUIT;
		E5150_DEBUG("from debugger {}", shouldStop);
	} while (!shouldStop);
	E5150_DEBUG("Emulator <-> debugger loop stopped");
}
