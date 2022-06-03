from io import FileIO
import cmd
import argparse
import commands

parser = argparse.ArgumentParser(description="Debugger CLI of E5150")
parser.add_argument("read_fifo_filename", help="Named pipe filename to read data from the emulator")
parser.add_argument("write_fifo_filename", help="Named pipe filename to write data to the emulator")
fromEmulator:FileIO = None
toEmulator:FileIO = None

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
			return commands.parse(fromEmulator,toEmulator,cmd)
		except:
			pass
		return False

	###########################
	#    Command functions    #
	###########################
	
	def do_continue(self, arg: str):
		"""continue the execution of the emulation"""
		return self._parse(f"continue {arg}")
	
	def do_step(self, arg: str):
		"""Step throw program execution"""
		return self._parse(f"step {arg}")
	
	def do_display(self, arg: str):
		"""Control display verbosity of the emulator"""
		return self._parse(f"display {arg}")
	
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

	fromEmulator = open(args.read_fifo_filename, "rb", buffering=0)
	toEmulator = open(args.write_fifo_filename, "wb", buffering=0)
	sizeof_pid_t = int.from_bytes(fromEmulator.read(1), byteorder="little")
	emulatorPID = int.from_bytes(fromEmulator.read(sizeof_pid_t), byteorder="little")

	print(f"[E5150 DEBUGGER]: Connected to emulator with PID {emulatorPID}")

	shell = DebuggerShell()

	while True:
		shell.cmdloop()
