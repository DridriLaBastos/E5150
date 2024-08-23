#include <cstdio>
#include <cerrno>
#include <cctype>

#include <semaphore>

#include "core/arch.hpp"

#if 0
#include "core/util.hpp"
#include "core/arch.hpp"

#include "debugger/commands.hpp"
#include "debugger/debugger.hpp"

#include "platform/platform.h"

using namespace E5150;

static unsigned int savedLogLevel = 0;
static bool debuggerInitialized = false;
static bool debuggerHasQuit = false;

static E5150::DEBUGGER::AbstractCommand* runningCommand = nullptr;
#endif

#include "core/debugger/debugger.hpp"
#include "core/debugger/commands.hpp"

#include "platform/platform.h"

static E5150::DEBUGGER::AbstractCommand* currentCommand = nullptr;
static std::vector<std::unique_ptr<E5150::DEBUGGER::AbstractCommand>> registeredCommands;

static struct
{
	/*COMMAND_TYPE*/ int type;

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
		type = 0;//COMMAND_TYPE_ERROR;
		passType = 0;
		subtype = 0;
		value = 0;
	}

} context;

void E5150::DEBUGGER::Init()
{
	registeredCommands.emplace_back(std::make_unique<COMMANDS::CommandContinue>());
	registeredCommands.emplace_back(std::make_unique<COMMANDS::CommandStep>());
#if 0
	context.clear();

#endif
}

std::binary_semaphore ready(0);
void E5150::DEBUGGER::Clean()
{
	//Release potentially waiting thread
	ready.release();
}


void E5150::DEBUGGER::WakeUp(const unsigned int cpuEvents)
{
	bool commandExecutionFinished = true;

	if (currentCommand)
	{
		commandExecutionFinished = currentCommand->Step(cpuEvents);
	}

	if(commandExecutionFinished)
	{
		currentCommand = nullptr;
		ready.acquire();
	}
}

static bool Launch(E5150::DEBUGGER::AbstractCommand* requestedCommand, const std::string& commandArgs)
{
	if (currentCommand) {
		spdlog::warn("Unable to launch the command '{}' because the command '{}' is already running",
		             requestedCommand->name.c_str(), currentCommand->name.c_str());
		return false;
	}

	//Prepare will return false with -h or --help option for command (or if any other error happened)
	//We don't want to Launch the command if it didn't get proper options
	const bool commandReady = requestedCommand->Parse(commandArgs);

	if (commandReady)
	{
		currentCommand = requestedCommand;
		ready.release();
	}

	return commandReady;
}

void E5150::DEBUGGER::ParseCmdLine(std::string line)
{
	const auto cmdNameBegin = std::find_if_not(line.begin(), line.end(),[](const char c) {
		return isspace(c);
	});

	const auto cmdNameEnd = std::find_if(cmdNameBegin, line.end(),[](const char c) {
		return isspace(c);
	});

	const std::string commandName (cmdNameBegin, cmdNameEnd);
	const std::string commandArgs (cmdNameEnd, line.end());

	const auto found = std::find_if(registeredCommands.begin(), registeredCommands.end(),
	                                [&commandName](const std::unique_ptr<E5150::DEBUGGER::AbstractCommand>& cmd) {
		                                return cmd->name == commandName;
	                                });

	if (found == registeredCommands.end())
	{
		spdlog::warn("[DEBUGGER] Command '{}' not found", commandName.c_str());
		return;
	}

	const bool cmdLaunchedSuccess = Launch(found->get(), commandArgs);

	if (!cmdLaunchedSuccess)
	{
		spdlog::warn("Cannot execute command '{}'",commandName);
	}
}

#if 0
static void OpenLockFile(bool initializeReadSide)
{
	const char* openMode = initializeReadSide ? "r" : "w";
	FILE** lockFile = initializeReadSide ? &lockFileRead : &lockFileWrite;
	*lockFile = fopen(FIFO_PATH(LOCK_FILE),openMode);

	if (*lockFile == nullptr) {
		spdlog::warn("[DEBUGGER]: Enable to open lock file to wait command. ERROR({}): '{}'",errno, strerror(errno));
		return;
	}

	const int ret = setvbuf(*lockFile, nullptr,_IONBF,0);

	if (ret == EOF) {
		spdlog::warn("[DEBUGGER]: Enable to modify buffering type");
		fclose(lockFileRead);
		return;
	}
	debuggerInitialized = true;
}

void E5150::DEBUGGER::PrepareSimulationSide()
{
	OpenLockFile(true);
}

void E5150::DEBUGGER::PrepareGuiSide() {
	OpenLockFile(false);
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
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu->decodedInst);
	if (!inst)
		return;
	std::cout << std::hex << workingState.CURRENT_INSTRUCTION_CS << ":" << workingState.CURRENT_INSTRUCTION_IP << " (" << cpu.genAddress(workingState.CURRENT_INSTRUCTION_CS,workingState.CURRENT_INSTRUCTION_IP) << ")" << std::dec << ": ";
	std::cout << xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(&cpu->decodedInst)) << " : length = " << xed_decoded_inst_get_length(&cpu->decodedInst) << std::endl;
	std::cout << xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(&cpu->decodedInst)) << " ";
	unsigned int realOperandPos = 0;
	bool foundPtr = false;

	for (unsigned int i = 0; i < xed_decoded_inst_noperands(&cpu->decodedInst); ++i)
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
				std::cout << (xed_decoded_inst_get_branch_displacement(&cpu->decodedInst) & 0xFFFF);
				break;

			case XED_OPERAND_PTR:
				std::cout << std::hex << (xed_decoded_inst_get_branch_displacement(&cpu->decodedInst) & 0xFFFF) << std::dec;
				foundPtr = true;
				break;

			case XED_OPERAND_REG0:
			case XED_OPERAND_REG1:
			case XED_OPERAND_REG2:
				std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_reg(&cpu->decodedInst, op_name));
				break;

			case XED_OPERAND_IMM0:
			case XED_OPERAND_IMM1:
				std::cout << std::hex << (xed_decoded_inst_get_unsigned_immediate(&cpu->decodedInst) & 0xFFFF) << std::dec;
				break;

			//Displaying memory operand with format SEG:[[BASE +] [INDEX +] DISPLACEMENT ]
			case XED_OPERAND_MEM0:
			{
				const xed_reg_enum_t baseReg = xed_decoded_inst_get_base_reg(&cpu->decodedInst, 0);
				const xed_reg_enum_t indexReg = xed_decoded_inst_get_index_reg(&cpu->decodedInst, 0);
				const int64_t memDisplacement = xed_decoded_inst_get_memory_displacement(&cpu->decodedInst,0);
				std::cout << ((xed_decoded_inst_get_memory_operand_length(&cpu->decodedInst, 0) == 1) ? "BYTE" : "WORD") << " ";
				std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_seg_reg(&cpu->decodedInst, 0)) << ":[";

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
		std::cout << " (iform: ' "<< xed_iform_enum_t2str(xed_decoded_inst_get_iform_enum(&cpu->decodedInst)) << "')";
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
#if 0
	context.type = 0;//COMMAND_TYPE_CONTINUE;
	context.passType = info.passType;
	context.count = info.passCount;

	savedLogLevel = E5150::Util::CURRENT_EMULATION_LOG_LEVEL;
	E5150::Util::CURRENT_EMULATION_LOG_LEVEL = 0;
#endif
}

static void handleStepCommand()
{
#if 0
	PassCommandInfo info;
	READ_FROM_DEBUGGER(&info,sizeof(PassCommandInfo));

	context.type = COMMAND_TYPE_STEP;
	context.passType = info.passType;
	context.count = 1;
#endif
}

static void handleDisplayCommand()
{
#if 0
	DisplayCommandInfo info;
	READ_FROM_DEBUGGER(&info, sizeof(DisplayCommandInfo));

	if (info.newLogLevel < 0)
	{
		E5150_INFO("Current log level : {}", E5150::Util::CURRENT_EMULATION_LOG_LEVEL);
		return;
	}

	if (info.newLogLevel > EMULATION_MAX_LOG_LEVEL)
	{
		E5150_INFO("Log level selected to high, log level applied will be the highest value : {}", EMULATION_MAX_LOG_LEVEL);
		E5150::Util::CURRENT_EMULATION_LOG_LEVEL = EMULATION_MAX_LOG_LEVEL;
		return;
	}

	E5150::Util::CURRENT_EMULATION_LOG_LEVEL = info.newLogLevel;
	E5150_INFO("Log level set to the new level {}", E5150::Util::CURRENT_EMULATION_LOG_LEVEL);
#endif
}

static void handleQuitCommand(void)
{ debuggerHasQuit = true; }

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
{ const auto& EUInfo = cpu.eu.getDebugWorkingState(); }

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
#if 0
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

		case PASS_TYPE_INSTRUCTION_STEP_THROUGH:
			E5150_WARNING("Instruction step throught command not yet implemented");
			return false;

		case PASS_TYPE_INFINITE:
			return true;

		default:
			E5150_WARNING("Unknown way to execute a pass command (continue/step). Command aborted");
			break;
	}
	return false;
#endif
	return true;
}

void E5150::DEBUGGER::wakeUp(const uint8_t instructionExecuted, const bool instructionDecoded)
{
	if (runningCommand) {
		const bool commandFinished = runningCommand->Step(instructionExecuted, instructionDecoded);
		if (commandFinished)
		{ runningCommand = nullptr; }
		return;
	}

	char wait;
	fread(&wait, sizeof(wait),1,lockFileRead);
}

bool E5150::DEBUGGER::Launch(const std::string &commandName, const std::string &commandArgs)
{
	if (runningCommand)
	{
		spdlog::warn("[DEBUGGER] Cannot launch two commands at the same time");
		return false;
	}

	const auto found = std::find_if(registeredCommands.begin(), registeredCommands.end(),
									[&commandName](const std::unique_ptr<AbstractCommand>& cmd) {
		return cmd->name == commandName;
	});

	if (found != registeredCommands.end())
	{
		//Prepare will return false with -h or --help option for command (or if any other error happened)
		//We don't want to Launch the command if it didn't get proper options
		const bool commandReady = (*found)->Parse(commandArgs);

		if (commandReady)
		{
			runningCommand = found->get();
			char commandReady = 0;
			const int status = fwrite(&commandReady,sizeof(commandReady),1,lockFileWrite);

			if (status < (int)sizeof(commandReady))
			{
				if (feof(lockFileWrite))
				{
					spdlog::warn("[DEBUGGER] Got EOF when unlocking the debugger for command execution... Thats unexpected");
				}

				if (ferror(lockFileWrite))
				{
					spdlog::warn("[DEBUGGER] Error when unlocking the debugger for command execution. ERROR({}) :'{}'", errno,
					             strerror(errno));
				}
			}
		}
	}

	return found != registeredCommands.end();
}
#endif
