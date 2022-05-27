import argparse
import ctypes
from io import FileIO

parser = argparse.ArgumentParser(description="Debug fonctions parser for the E5150 debugger")

### Continue command arguments

subparser = parser.add_subparsers(title="functions", dest="command")
continue_parser = subparser.add_parser("continue", aliases=["cont", "c"], description="Continue the execution of the emulation", help="Continue the emulation")
continue_args = continue_parser.add_mutually_exclusive_group()
continue_args.add_argument("-i", "--pass-instructions", type=int, default=-1, help="The number of instructions to execute before stoping again", metavar="#INSTRUCTIONS")
continue_args.add_argument("-c", "--pass-clocks", type=int, default=-1, help="The number of clock to executes before stoping again", metavar="#CLOCKS")
continue_args.add_argument("-b", "--pass-bus_cycles", type=int, default=-1, help="The number of bus cycles to pass before stoping again", metavar="#BUS_CYCLES")

### Step command arguments

step_parser = subparser.add_parser("step", aliases=["s"], help="one step into the emulation")
step_args = step_parser.add_mutually_exclusive_group()
step_args.add_argument("-i", "--instruction", action="store_true", help="Execute one instruction and stop after the instruction ends")
step_args.add_argument("-c", "--clock", action="store_true", help="Execute one clock")
step_args.add_argument("-b", "--bus-cycle", action="store_true", help="Execute one bus cycle and stop abefore a new one begins")

### Display command arguments

display_parser = subparser.add_parser("display", aliases=["disp", "d"], help="Change emulation display rules", description="Change the behaviour when diplaying information of the emulation. Each arguments acts as a flags and it is possible to use severals of them to toggle differents informations display. If no arguments is provided all displays are disabled")
display_parser.add_argument("-i", "--instructions", action="store_true", help="Toggle displaying executed instruction")
display_parser.add_argument("-r", "--registers", action="store_true", help="Toggle displaying cpu registers")
display_parser.add_argument("-f", "--flags", action="store_true", help="Toggle displaying cpu flags")
display_parser.add_argument("-l", "--log-level",metavar="LEVEL",type=int,help="The debugging message log level. Higher means more log and smaller means less log. The value must be positive")

### Quit the debugger
quit_parser = subparser.add_parser("quit", aliases="q", help="Quit the emulation", description="Quit the emulation")

_com: ctypes.CDLL = None

parseOK = False

def parse(fromEmulator: FileIO, toEmulator: FileIO, command: str) -> bool:
	global _com
	global parseOK
	if not _com:
		_com = ctypes.CDLL('/Users/adrien/Documents/Informatique/C++/E5150/build/debugger/communication/libdecom.dylib')
		_com.registerCommunicationFifos(fromEmulator.fileno(), toEmulator.fileno())
	chunks = command.split(" ")
	parseOK = False

	result = parser.parse_args(chunks)
	parseOK = True
	# print(result)
	if result.command == "continue":
		return _com.sendContinueCommandInfo(result.pass_instructions, result.pass_clocks, result.pass_bus_cycles)
	elif result.command == "step":
		return _com.sendStepCommandInfo()
	elif result.command == "display":
		return _com.sendDisplayCommandInfo(result.flags, result.instructions, result.registers, -1 if result.log_level is None else result.log_level)

	return _com.sendUnknownCommandInfo()
