import argparse
import logging
import os
import signal
from sys import getsizeof

cont = True
parser = argparse.ArgumentParser(description="Debug fonctions parser for the E5150 debugger")

### Continue command arguments

subparser = parser.add_subparsers(title="functions", dest="command")
continue_parser = subparser.add_parser("continue", aliases=["c", "cont"], description="Continue the execution of the emulation", help="Continue the emulation")
continue_args = continue_parser.add_mutually_exclusive_group()
continue_args.add_argument("-i", "--pass-instructions", type=int, default=-1, help="The number of instructions to execute before stoping again", metavar="#INSTRUCTIONS")
continue_args.add_argument("-c", "--pass-clock",type=int, default=-1, help="The number of clock to executes before stoping again", metavar="#CLOCKS")
continue_args.add_argument("-b", "--pass-bus_cycles",type=int, default=-1, help="The number of bus cycles to pass before stoping again", metavar="#BUS_CYCLES")

### Step command arguments

step_parser = subparser.add_parser("step", aliases="s", help="one step into the emulation")
step_args = step_parser.add_mutually_exclusive_group()
step_args.add_argument("-i", "--instruction", action="store_true", help="Execute one instruction and stop after the instruction ends")
step_args.add_argument("-c", "--clock", action="store_true", help="Execute one clock")
step_args.add_argument("-b", "--bus-cycle", action="store_true", help="Execute one bus cycle and stop abefore a new one begins")

### Display command arguments

display_parser = subparser.add_parser("display", aliases="d", help="Change emulation display rules", description="Change the behaviour when diplaying information of the emulation. Each arguments acts as a flags and it is possible to use severals of them to toggle differents informations display. If no arguments is provided all displays are disabled")
display_parser.add_argument("-i", "--instructions", action="store_true", help="Toggle displaying executed instruction")
display_parser.add_argument("-r", "--registers", action="store_true", help="Toggle displaying cpu registers")
display_parser.add_argument("-f", "--flags", action="store_true", help="Toggle displaying cpu flags")
display_parser.add_argument("-l", "--log-level",metavar="LEVEL",type=int,help="The debugging message log level. Higher means more log and smaller means less log. The value must be positive")

### Quit the debugger
quit_parser = subparser.add_parser("quit", aliases="q", help="Quit the emulation", description="Quit the emulation")

instructionExecCount = 0

print(f"[DEBUGGER]: Running !")

fromEmulator = open("/Users/adrien/Documents/Informatique/C++/E5150/build/make/macos/.ed.fifo", "rb", buffering=0)
toEmulator = open("/Users/adrien/Documents/Informatique/C++/E5150/build/make/macos/.de.fifo", "wb", buffering=0)
emulatorPID = int.from_bytes(fromEmulator.read(4), byteorder="little")

print(f"[DEBUGGER] Connected to emulator (PID {emulatorPID})")

lastCmd = ""
dataToEmulator = None
while True:
	goodCommand = False
	instructionExecCount += int.from_bytes(fromEmulator.read(1),byteorder='little')
	while not goodCommand:
		userCmd = input(f"({instructionExecCount}) > ")

		if len(userCmd) == 0:
			userCmd = lastCmd

		chunks = userCmd.split(" ")

		try:
			nm = parser.parse_args(chunks)
			goodCommand = True
			lastCmd = userCmd
			toEmulator.write(b"\x00")
			print(nm)
		except:
			pass