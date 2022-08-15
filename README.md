# E5150

This is an emulator of the first PC of the history : the IBM 5150.
It is running with an intel 8088, and this emulator is an to recreate it
in the most fidel way.

The emulator also provides a debugger. For now it can only be used to step through
clocks and instructions with some insight messages, but in the future it will be complete
debugger with the ability to read/write registers, of the processor **AND** other components
of the PC. The goal is for the debugger to be an interface to every elements og the PC.

# Getting started
First, clone the repository with `git clone -j <number of threads> -b dev --recursive https://github.com/dridrilabastos/E5150`,
then go to the created directory. The rest of this guide assumes that you are in the folder created by the `git clone` command.

**Warning** : The emulator uses an external library to decode instructions. To build this library
it is necessary to have a python interpreter installed.

**Warning** : On Windows, don't build in debug mode, I have still some strange bugs. To have Debug symbols,
build on RelWithDebInfo. There is still some errors when executing on Windows. When the error window pop in, in Visual Studio
press the continue button multiple time on the top bar. The program will still run fine. Investigations in progress...

**Warning** : 15/08/2022 : For an unknown reason, there are link errors when building on Windows with CLion, but not when
building with Visual Studio, and only in 1 test PC out of the 3 I have.

## Build - the geek way
Run `cmake -B <build path> -j <threads> -DCMAKE_BUILD_TYPE=<[Release|Debug|RelWithDebInfo]> --target run` to build and run the emulator.
By adding the `-G` parameter to the cmake line, it is possible to select the generator.

## Build - IDE
Some IDEs support Cmake : open the folder created by the clone command with IDEs 
like vscode (with microsoft CMake extension), Visual Studio or Clion, and the project structure will be automatically
detected.

## Build the debugger
The debugger is built by default, to remove it from the build step, you can
change line 15: `option(ENABLE_DEBUGGER "Compile the integrated debugger" ON)`
to `option(ENABLE_DEBUGGER "Compile the integrated debugger" OFF)` in the root
CMakeLists.txt file and run the build command again.

You can also add the option `-DENABLE_DEBUGGER=OFF` to the build command.

**Warning** : When building with CLion, be sure to set the toolchain to Visual Studio. I have some work left to port
windows platform dependent code to MinGW.

## Troubleshoot
 * nasm no found : comment the line 3 to 6 and and 24 to 44 in `test/CMakeLists.txt`
