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
add_compile_definitions(DEBUGGER_ON)
endif()

if (NOT WIN32)
	add_compile_options(-Wno-switch -Wall -Wextra)
else ()
add_link_options("$<$<CONFIG:RelWithDebInfo>:/PROFILE>")
endif()

include_directories(${CMAKE_SOURCE_DIR} third-party third-party/spdlog/include third-party/argparse/include third-party/imgui ${CMAKE_BINARY_DIR}/third-party/include)

add_subdirectory(third-party)
add_subdirectory(test)
add_subdirectory(gui)

#Maybe using a configuration file later
#configure_file(config.hpp.in config.hpp)

file(GLOB CORE_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/src/*.cpp")
file(GLOB CPU_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/src/cpu/*.cpp")
file(GLOB FLOPPY_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/src/floppy/*.cpp")

set(EMULATOR_SRC ${CORE_SRC} ${CPU_SRC} ${FLOPPY_SRC})

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
	add_subdirectory(debugger)

	set(DEBUGGER_SRC_PATH ${PROJECT_SOURCE_DIR}/debugger)

	target_sources(epc PRIVATE ${DEBUGGER_SRC} ${PLATFORM_SRC})
	# TODO: gives this as install path
	target_compile_definitions(epc PRIVATE DEBUGGER_SERVER_EXE="${DEBUGGER_SERVER_EXE}")

	add_dependencies(epc debugger-server)
	target_link_libraries(epc decom)
endif()

add_custom_target(run ${PROJECT_BINARY_DIR}/epc
					WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
					DEPENDS epc)

if(WIN32)
	add_custom_command(TARGET epc POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:imgui> $<TARGET_FILE_DIR:epc>)
endif()
