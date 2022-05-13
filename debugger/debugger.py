from email.policy import default
from io import FileIO
import cmd
import argparse
import commands

parser = argparse.ArgumentParser(description="Debugger CLI of E5150")
parser.add_argument("read_fifo_filename", help="Named pipe filename to read data from the emulator")
parser.add_argument("write_fifo_filename", help="Named pipe filename to write data to the emulator")
instructionExecCount = 0


class DebuggerShell(cmd.Cmd):
	intro = ""
	prompt = ""
	lastCmd = ""
	use_rawinput = True
	fromEmulator: FileIO = None
	toEmulator: FileIO = None

	def cmdloop(self) -> None:
		global instructionExecCount
		instructionExecCount += int.from_bytes(fromEmulator.read(1),byteorder="little")
		self.prompt = f"({instructionExecCount}) > "
		return super().cmdloop()
	
	def _parse(self, cmd: str):
		try:
			return commands.parse(self.fromEmulator, self.toEmulator, cmd)
		except:
			pass
		return False
	
	def do_continue(self, arg: str):
		"""continue the execution of the emulation"""
		return self._parse(f"continue {arg}")
	
	def do_help(self, arg: str) -> bool:
		"""Show this help"""
		return super().do_help(arg) if not arg else self._parse(arg + " --help")

if __name__ == "__main__":
	args = parser.parse_args()
	print("[E5150 DEBUGGER]: Debugger is running!")

	fromEmulator = open(args.read_fifo_filename, "rb", buffering=0)
	toEmulator = open(args.write_fifo_filename, "wb", buffering=0)
	emulatorPID = int.from_bytes(fromEmulator.read(4), byteorder="little")

	shell = DebuggerShell()
	shell.fromEmulator = fromEmulator
	shell.toEmulator = toEmulator

	print(f"[E5150 DEBUGGER]: Connected to emulator with PID {emulatorPID}")

	while True:
		DebuggerShell().cmdloop()
		toEmulator.write(b"\x00")
