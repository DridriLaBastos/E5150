from io import FileIO
import cmd
import argparse
import commands
import os

parser = argparse.ArgumentParser(description="Debugger CLI of E5150")
parser.add_argument("read_fifo_filename", help="Named pipe filename to read data from the emulator")
parser.add_argument("write_fifo_filename", help="Named pipe filename to write data to the emulator")
parser.add_argument("decom_path", help="Path to the library needed for the communication from the debugger to the emulator")
fromEmulator:FileIO = None
toEmulator:FileIO = None
decomPath: str = ""

class DebuggerShell(cmd.Cmd):
	intro = ""
	use_rawinput = True

	###########################
	# Override core functions #
	###########################
	def cmdloop(self) -> None:
		global instructionExecCount
		instructionExecCount = int.from_bytes(fromEmulator.read(8),byteorder="little")
		self.prompt = f"({instructionExecCount}) > "
		return super().cmdloop()
	
	def postcmd(self, stop: bool, line: str) -> bool:
		if commands.parseOK:
			toEmulator.write(b'\x01' if stop else b'\x00')
		return super().postcmd(stop, line)
	
	def default(self, line: str) -> None:
		return self._parse(line)
	
	###########################
	#    Utility functions    #
	###########################

	def _parse(self, cmd: str):
		try:
			return commands.parse(fromEmulator,toEmulator,decomPath,cmd)
		except:
			pass
		return False
	
	def do_help(self, arg: str) -> bool:
		"""Show this help"""
		if not arg:
			self._parse("--help")
		else:
			self._parse(arg + " --help")
		return False

def shellLoop():
	global instructionExecCount
	global fromEmulator
	global toEmulator
	exitLoop = False
	lastExecutedCmd = None
	instructionExecCount += int.from_bytes(fromEmulator.read(8),byteorder="little")
	while not exitLoop:
		cmd = input(f"({instructionExecCount}) > ")

		if not cmd and lastExecutedCmd:
			cmd = lastExecutedCmd

		exitLoop = commands.parse(fromEmulator,toEmulator,cmd)
		lastExecutedCmd = cmd

	toEmulator.write(b'\x01' if exitLoop else b'\x00')

if __name__ == "__main__":
	args = parser.parse_args()
	print("[E5150 DEBUGGER]: Debugger is running!")
	print(f"[E5150 DEBUGGER]: called with decom = {args.decom_path}")

	runningOnWindows = os.name == 'nt'
	pipePathPrefix = '//./pipe/' if runningOnWindows else ''

	fromEmulatorFifoFileName = f"{pipePathPrefix}{args.read_fifo_filename}"
	toEmulatorFifoFileName = f"{pipePathPrefix}{args.write_fifo_filename}"

	print(f"computed path: ed '{fromEmulatorFifoFileName}'   de '{toEmulatorFifoFileName}'")

	fromEmulator = open(fromEmulatorFifoFileName, "rb", buffering=0)
	toEmulator = open(toEmulatorFifoFileName, "wb", buffering=0)
	decomPath = args.decom_path
	synchronizationData = int.from_bytes(fromEmulator.read(4), byteorder="little")

	if synchronizationData == 0xDEAB12CD :
		print(f"[E5150 DEBUGGER]: Connected to emulator")
	else:
		print(f"[E5150 DEBUGGER]: Wrong synchronization data, expected 0xDEAB12CD, got 0x{synchronizationData:x}")

	shell = DebuggerShell()

	while True:
		shell.cmdloop()

