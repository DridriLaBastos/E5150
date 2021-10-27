cmake_minimum_required(VERSION 3.10)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(E5150
		VERSION 0.1
		DESCRIPTION "Clock accurate emulator of the IBM PC 5150"
		LANGUAGES C CXX ASM_NASM)

set(CMAKE_BUILD_TYPE "Debug")

#Maybe using a configuration file later
#configure_file(config.hpp.in config.hpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/third-party/spdlog/include ${PROJECT_SOURCE_DIR})

add_subdirectory(core)
add_subdirectory(e5150)

add_custom_target(run
					COMMAND epc.out
					WORKING_DIRECTORY e5150
				)
