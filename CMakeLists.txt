# target_precompiled_headers available from 3.16
cmake_minimum_required(VERSION 3.16)

project(E5150
		VERSION 0.1
		DESCRIPTION "Clock accurate emulator of the IBM PC 5150"
		LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

option(ENABLE_DEBUGGER "Compile the integrated debugger" ON)

if (ENABLE_DEBUGGER)
add_compile_definitions(DEBUGGER)
endif()

if (NOT WIN32)
	add_compile_options(-Wno-switch)
endif()

if (WIN32)
add_link_options("$<$<CONFIG:RelWithDebInfo>:/PROFILE>")
endif()

include_directories(${CMAKE_SOURCE_DIR} third-party/spdlog/include third-party third-party/imgui ${CMAKE_BINARY_DIR}/third-party/include)

add_subdirectory(third-party)
add_subdirectory(test)
add_subdirectory(gui)

#Maybe using a configuration file later
#configure_file(config.hpp.in config.hpp)

file(GLOB CORE_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/src/*.cpp")
file(GLOB CPU_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/src/cpu/*.cpp")
file(GLOB FLOPPY_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/src/floppy/*.cpp")

set(EMULATOR_SRC ${CORE_SRC} ${CPU_SRC} ${FLOPPY_SRC})
set(DEBUGGER_SRC_PATH ${PROJECT_SOURCE_DIR}/debugger)

if(UNIX)
	set(PLATFORM_SRC platform/platform_unix.c)
elseif(WIN32)
	set(PLATFORM_SRC platform/platform_win32.c)
else()
	message(FATAL_ERROR "Architecture ${CMAKE_SYSTEM} not recognized, cannot build debugger platform dependent code.")
endif()

# Adding the header only files as sources to make theme appear in IDE, only using source_group isn't enougth if th source file isn't used
add_executable(epc main.cpp ${EMULATOR_SRC} ${PLATFORM_SRC})
add_dependencies(epc asm-test)

target_link_libraries(epc ${XED_LIB_PATH} imgui gui)
target_precompile_headers(epc PUBLIC $<$<COMPILE_LANGUAGE:CXX>:core/pch.hpp>)

if (ENABLE_DEBUGGER)
	find_package(Python3 COMPONENTS Interpreter REQUIRED)

	if(UNIX)
		set(DEBUGGER_PLATFORM_STRING "unix")
	elseif(WIN32)
		set(DEBUGGER_PLATFORM_STRING "win32")
	else()
		message(FATAL_ERROR "Architecture ${CMAKE_SYSTEM} not recognized, cannot build debugger platform dependent code.")
	endif()

	target_sources(epc PRIVATE ${DEBUGGER_SRC_PATH}/debugger.cpp ${DEBUGGER_SRC_PATH}/gui.cpp ${DEBUGGER_SRC_PATH}/communication/communication.c ${PLATFORM_SRC})
	# TODO: gives this as install path
	target_compile_definitions(epc	PRIVATE DECOM_LIB_PATH="$<TARGET_FILE:decom>"
									PRIVATE DEBUGGER_PYTHON_SCRIPT_PATH="${PROJECT_SOURCE_DIR}/debugger/debugger.py"
									PRIVATE DEBUGGER_PLATFORM_STRING="${DEBUGGER_PLATFORM_STRING}"
									PRIVATE PYTHON3_EXECUTABLE_PATH="${Python3_EXECUTABLE}")

	add_library(decom SHARED "${DEBUGGER_SRC_PATH}/communication/command.c" "${DEBUGGER_SRC_PATH}/communication/communication.c" ${PLATFORM_SRC})
	add_dependencies(epc decom)
endif()

add_custom_target(run ${PROJECT_BINARY_DIR}/epc
					WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
					DEPENDS epc)
