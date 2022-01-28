#include <iostream>

#include "util.hpp"
#include "arch.hpp"
#include "debugger.hpp"

using namespace E5150;

static constexpr size_t USER_COMMAND_BUFFER_SIZE = 128;

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

static void printDebugInfo (const bool instructionExecuted)
{
	switch (state.clockBehaviour)
	{
		case CLOCK_BEHAVIOUR::PASS:
			break;
		
		case CLOCK_BEHAVIOUR::ALWAYS_STOP:
		{
			cpu.biu.debugClockPrint();
			cpu.eu.debugClockPrint();
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

void Debugger::wakeUp(const bool instructionExecuted)
{
	printDebugInfo(instructionExecuted);
	printDebuggerCLI(instructionExecuted);
}