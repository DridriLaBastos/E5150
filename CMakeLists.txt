cmake_minimum_required(VERSION 3.10)

project(E5150
		VERSION 0.1
		DESCRIPTION "Clock accurate emulator of the IBM PC 5150"
		LANGUAGES C CXX)

add_subdirectory(third-party)
add_subdirectory(test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

option(ENABLE_DEBUGGER "Compile the integrated debugger" ON)

if (NOT MSVC)
	add_compile_options(-Wno-switch)
endif()

#Maybe using a configuration file later
#configure_file(config.hpp.in config.hpp)

file(GLOB CORE_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/src/*.cpp")
file(GLOB CPU_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/src/cpu/*.cpp")
file(GLOB FLOPPY_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/src/floppy/*.cpp")

file(GLOB CORE_HEADERS "${PROJECT_SOURCE_DIR}/core/include/*.hpp")
set_source_files_properties(${CORE_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES main.cpp ${CORE_SRC} ${CORE_HEADERS})

set(EMULATOR_SRC ${CORE_SRC} ${CPU_SRC} ${FLOPPY_SRC})

# Addind the header only files as sources to make theme appear in IDE, only using source_group isn't enougth if th source file isn't used
add_executable(epc main.cpp ${EMULATOR_SRC} ${EMULATOR_HEADERS})
add_dependencies(epc asm-test)

target_include_directories(epc PUBLIC ${PROJECT_SOURCE_DIR}/core/include PUBLIC ${PROJECT_SOURCE_DIR}/third-party/spdlog/include PUBLIC ${PROJECT_SOURCE_DIR}/third-party/include PUBLIC ${PROJECT_BINARY_DIR}/third-party/include)
target_link_libraries(epc ${XED_LIB_PATH})
target_precompile_headers(epc PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_SOURCE_DIR}/core/include/pch.hpp>)

if (ENABLE_DEBUGGER)
	set(DEBUGGER_SRC_PATH ${PROJECT_SOURCE_DIR}/debugger)

	if(UNIX)
		set(PLATFORM_SRC ${DEBUGGER_SRC_PATH}/platform/platform_unix.c)
		set(DEBUGGER_PLATFORM_STRING "unix")
	elseif(WIN32)
		set(PLATFORM_SRC ${DEBUGGER_SRC_PATH}/platform/platform_win32.c)
		set(DEBUGGER_PLATFORM_STRING "win32")
	else()
		message(FATAL_ERROR "Architecture not recognized, cannot build debugger platform dependent code.")
	endif()

	find_package(Python3 COMPONENTS Interpreter REQUIRED)

	target_sources(epc PRIVATE ${DEBUGGER_SRC_PATH}/debugger.cpp ${PLATFORM_SRC})
	target_compile_definitions(epc PRIVATE PYTHON3_EXECUTABLE_PATH="${Python3_EXECUTABLE}")
	target_compile_definitions(epc PRIVATE DEBUGGER)
	target_include_directories(epc PRIVATE ${DEBUGGER_SRC_PATH})

	add_library(decom SHARED ${DEBUGGER_SRC_PATH}/communication/command.c ${DEBUGGER_SRC_PATH}/communication/communication.c ${PLATFORM_SRC})
	target_include_directories(decom PRIVATE ${DEBUGGER_SRC_PATH})

	# TODO: gives this as install path
	target_compile_definitions(epc	PRIVATE DECOM_LIB_PATH="$<TARGET_FILE:decom>"
									PRIVATE DEBUGGER_PYTHON_SCRIPT_PATH="${PROJECT_SOURCE_DIR}/debugger/debugger.py"
									PRIVATE DEBUGGER_PLATFORM_STRING="${DEBUGGER_PLATFORM_STRING}")

endif()

add_custom_target(run ${PROJECT_BINARY_DIR}/epc
					WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
					DEPENDS epc)
