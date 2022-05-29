import argparse
from ast import alias
import chunk
import ctypes
from io import FileIO

parser = argparse.ArgumentParser(description="Debug fonctions parser for the E5150 debugger")

commands = {
	"continue": ["continue", "cont","c"],
	"stepi": ["stepi","si"],
	"stepo": ["stepo", "so", "next", "n"],
	"stepc": ["stepc", "sc"],
	"display": ["display", "disp", "d"],
	"quit": ["quit", "exit", "q"]
}

### Continue command arguments

subparser = parser.add_subparsers(title="functions", dest="command")
continue_parser = subparser.add_parser(commands["continue"][0], aliases=commands["continue"][1:], description="Continue the execution of the emulation", help="Continue the emulation")
continue_args = continue_parser.add_mutually_exclusive_group()
continue_args.add_argument("-i", "--pass-instructions", type=int, default=-1, help="The number of instructions to execute before stoping again", metavar="#INSTRUCTIONS")
continue_args.add_argument("-c", "--pass-clocks", type=int, default=-1, help="The number of clock to executes before stoping again", metavar="#CLOCKS")

### Step out command arguments

step_parser = subparser.add_parser("step", help="running one step of the emulation", description="running one step of the emulation")
step_args = step_parser.add_mutually_exclusive_group()
step_args.add_argument("-i", "--instruction", action="store_true", help="[default] step of one instruction", default=True)
step_args.add_argument("-c", "--clock", action="store_true", help="step of one clock", default=False)
#pass is reserved word of python using _pass instead
step_parser.add_argument("-p", "--pass", dest="_pass", action="store_true", help="on instruction step mode, if a control transfer instruction is stepped, don't follow the instruction and go to the next linear instruction")

subparser.add_parser(commands["stepi"][0], aliases=commands["stepi"][1:], help="step into control transfer [`shortcut for step`]", description="runnig one step of the emulation following control transfer instructions. This command is shortcut for the `step` command. For more informations see `help step`")
subparser.add_parser(commands["stepo"][0], aliases=commands["stepo"][1:], help="step outof control transfer [`shortcut for step`]", description="runnig one step of the emulation passing a control transfer instructions. This command is shortcut for the `step` command. For more informations see `help step`")
subparser.add_parser(commands["stepc"][0], aliases=commands["stepc"][1:], help="clock step [shortcut for `step`]", description="perform one clock of the main clock generator. This command is shortcut for the `step` command. For more informations see `help step`")

### Display command arguments

display_parser = subparser.add_parser(commands["display"][0], aliases=commands["display"][1:], help="Change emulation display rules", description="Change the behaviour when diplaying information of the emulation. Each arguments acts as a flags and it is possible to use severals of them to toggle differents informations display. If no arguments is provided all displays are disabled")
display_parser.add_argument("-i", "--instructions", action="store_true", help="Toggle displaying executed instruction")
display_parser.add_argument("-r", "--registers", action="store_true", help="Toggle displaying cpu registers")
display_parser.add_argument("-f", "--flags", action="store_true", help="Toggle displaying cpu flags")
display_parser.add_argument("-l", "--log-level",metavar="LEVEL",type=int,help="The debugging message log level. Higher means more log and smaller means less log. The value must be positive")

### Quit the debugger
quit_parser = subparser.add_parser(commands["quit"][0], aliases=commands["quit"][1:], help="Quit the emulation", description="Quit the emulation")

_com: ctypes.CDLL = None

parseOK = False

def parse(fromEmulator: FileIO, toEmulator: FileIO, command: str) -> bool:
	global _com
	global parseOK
	if not _com:
		_com = ctypes.CDLL('/Users/adrien/Documents/Informatique/C++/E5150/build/debugger/communication/libdecom.dylib')
		_com.registerCommunicationFifos(fromEmulator.fileno(), toEmulator.fileno())
	chunks = command.split()
	parseOK = False
	result = parser.parse_args(chunks)
	parseOK = True
	if result.command in commands["continue"]:
		return _com.sendContinueCommandInfo(result.pass_instructions, result.pass_clocks)
	elif result.command == "step":
		print("[E5150 DEBUGGER] REACHED")
		if result.clock:
			result.instruction = False
		print(result)
		return _com.sendStepCommandInfo(result.instruction, result.clock, result._pass)
	elif result.command in commands["stepi"]:
		return _com.sendStepCommandInfo(True, False, False)
	elif result.command in commands["stepo"]:
		return _com.sendStepCommandInfo(True, False, True)
	elif result.command in commands["stepc"]:
		return _com.sendStepCommandInfo(False, True, False)
	elif result.command in commands["display"]:
		return _com.sendDisplayCommandInfo(result.flags, result.instructions, result.registers, -1 if result.log_level is None else result.log_level)

	return _com.sendUnknownCommandInfo()
