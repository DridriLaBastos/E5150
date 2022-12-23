import time
from io import FileIO
import cmd
import argparse
import sys
import commands
import os
import ctypes

parser = argparse.ArgumentParser(description="Debugger CLI of E5150")
parser.add_argument("read_fifo_filename", help="Named pipe filename to read data from the emulator")
parser.add_argument("write_fifo_filename", help="Named pipe filename to write data to the emulator")
parser.add_argument("decom_path",
                    help="Path to the library needed for the communication from the debugger to the emulator")
fromEmulator: FileIO = None
toEmulator: FileIO = None
decomPath: str = ""
decom: ctypes.CDLL = None


class DebuggerShell(cmd.Cmd):
	intro = ""
	use_rawinput = True
	prompt = " > "

	###########################
	# Override core functions #
	###########################
	# def cmdloop(intro) -> None:
	# 	# TODO: This function is called only one time so the instruction exec count is called at the begining
	# 	# but the emulator send it everytime and thus a shift happen
	# 	print("cmdloop")
	# 	instructionExecCount = ctypes.c_int64(0)
	# 	decom.readFromRegisteredDest(ctypes.byref(instructionExecCount), ctypes.sizeof(ctypes.c_int64))
	# 	self.prompt = f"({instructionExecCount.value}) > "
	# 	return super().cmdloop()

	def postcmd(self, stop: bool, line: str) -> bool:
		if stop is None:
			return False
		stopi8 = ctypes.c_uint8(stop)
		stopi8Ptr = ctypes.pointer(stopi8)
		decom.writeToRegisteredDest(stopi8Ptr, 1)
		tmp = ctypes.c_uint8(0)
		decom.readFromRegisteredDest(ctypes.byref(tmp), 1)  # Emulator finishes to execute the command
		return commands.shouldQuit

	def default(self, line: str) -> bool:
		return self._parse(line)

	###########################
	#    Utility functions    #
	###########################

	def _parse(self, cmd: str):
		try:
			return commands.parse(fromEmulator, toEmulator, decomPath, cmd)
		except:
			pass
		return False

	def do_help(self, arg: str) -> bool:
		"""Show this help"""
		if not arg:
			self._parse("--help")
		else:
			self._parse(arg + " --help")
		return None


if __name__ == "__main__":
	args = parser.parse_args()
	print("[E5150 DEBUGGER]: Debugger is running!")
	print(f"[E5150 DEBUGGER]: called with decom = {args.decom_path}")

	runningOnWindows = os.name == 'nt'
	pipePathPrefix = '//./pipe/' if runningOnWindows else ''

	fromEmulatorFifoFileName = f"{pipePathPrefix}{args.read_fifo_filename}"
	toEmulatorFifoFileName = f"{pipePathPrefix}{args.write_fifo_filename}"

	fromEmulator = open(fromEmulatorFifoFileName, "rb", buffering=0)
	toEmulator = open(toEmulatorFifoFileName, "wb", buffering=0)

	decom = ctypes.CDLL(args.decom_path)
	commands._decom = decom
	decom.registerCommunicationFifos(fromEmulator.fileno(), toEmulator.fileno())

	synchronizationData = ctypes.c_uint(0)

	decom.readFromRegisteredDest(ctypes.byref(synchronizationData), ctypes.sizeof(synchronizationData))

	if synchronizationData.value != 0xDEAB12CD:
		sys.exit(f"[E5150 DEBUGGER]: Wrong synchronization data, expected 0xDEAB12CD, got 0x{synchronizationData.value:X}")

	print(f"[E5150 DEBUGGER]: Connected to emulator")
	#shell = DebuggerShell()

	#TODO: review tgis while True loop, I want everything o be treated inside the cmdloop function
	while True:
		commandLineLength = ctypes.c_size_t(0)
		decom.readFromRegisteredDest(ctypes.byref(commandLineLength),ctypes.sizeof(commandLineLength))

		cmdLine = ctypes.create_string_buffer(commandLineLength.value)
		decom.readFromRegisteredDest(ctypes.byref(cmdLine),commandLineLength)

		try:
			commands.parse(fromEmulator,toEmulator,decomPath,cmdLine.raw.decode("ascii"))
		except:
			pass
