cmake_minimum_required(VERSION 3.10)

project(E5150
		VERSION 0.1
		DESCRIPTION "Clock accurate emulator of the IBM PC 5150"
		LANGUAGES C CXX ASM_NASM)

add_compile_options(-Wno-switch)

set(CMAKE_XCODE_SCHEME_WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/e5150)
set(CMAKE_XCODE_SCHEME_WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/e5150)

#Maybe using a configuration file later
#configure_file(config.hpp.in config.hpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/third-party/spdlog/include ${PROJECT_SOURCE_DIR})

if (CMAKE_HOST_UNIX)
add_compile_options($<$<CONFIG:RELEASE>:-flto>)
add_link_options   ($<$<CONFIG:RELEASE>:-flto>)
endif()

add_subdirectory(core)
add_subdirectory(e5150)

add_custom_target(run
					COMMAND epc.out
					WORKING_DIRECTORY e5150
				)
