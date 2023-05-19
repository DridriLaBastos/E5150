# target_precompiled_headers available from 3.16
cmake_minimum_required(VERSION 3.16)

if (APPLE)
	set(PLATFORM_LANGUAGES OBJC OBJCXX)
endif()

project(E5150
		VERSION 0.1
		DESCRIPTION "Clock accurate emulator of the IBM PC 5150"
		LANGUAGES C CXX ${PLATFORM_LANGUAGES})

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
set(PLATFORM_SRC platform/platform_unix.c)

if(APPLE)
	file(GLOB GUI_PLATFORM_SRC CONFIGURE_DEPENDS gui/macos/*.mm gui/macos/*.cpp)
elseif(WIN32)
	set(PLATFORM_SRC platform/platform_win32.c)
	file(GLOB GUI_PLATFORM_SRC CONFIGURE_DEPENDS gui/win/gui_d3d11.cpp)
	if (HOT_RELOAD_PDB_NAME)
		message(INFO "Build with PDB name ${HOT_RELOAD_PDB_NAME}")
		set_property(TARGET hot_reload_draw PROPERTY PDB_NAME ${HOT_RELOAD_PDB_NAME})
	endif()
else()
	message(FATAL_ERROR "Architecture ${CMAKE_SYSTEM} not recognized, cannot build platform dependent code")
endif()

add_executable(epc main.cpp ${EMULATOR_SRC} ${PLATFORM_SRC} ${GUI_PLATFORM_SRC} ${CMAKE_SOURCE_DIR}/gui/common/imgui_common.cpp)
target_precompile_headers(epc PUBLIC $<$<COMPILE_LANGUAGE:CXX,OBJC,OBJCXX>:core/pch.hpp>)

target_compile_definitions(epc PRIVATE DRAW_LIBRARY_FULL_PATH="$<TARGET_FILE:hot_reload_draw>"
		DRAW_LIBRARY_PREFIX_BASE_NAME="$<TARGET_FILE_PREFIX:hot_reload_draw>$<TARGET_FILE_BASE_NAME:hot_reload_draw>"
		DRAW_LIBRARY_FILE_EXTENSION="$<TARGET_FILE_SUFFIX:hot_reload_draw>")
add_dependencies(epc asm-test)

target_link_libraries(epc ${XED_LIB_PATH} imgui)

#TODO: Maybe I want the macro DEBUGGER_ON to be defined inside debugger.hpp ?
if (ENABLE_DEBUGGER)
	target_sources(epc PUBLIC debugger/debugger.cpp debugger/cli.cpp)
	target_compile_definitions(epc PRIVATE DEBUGGER_ON)
	target_compile_definitions(hot_reload_draw PRIVATE DEBUGGER_ON)
endif()

add_custom_target(run ${PROJECT_BINARY_DIR}/epc
					WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
					DEPENDS epc)

if(WIN32)
	add_custom_command(TARGET epc POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:imgui> $<TARGET_FILE_DIR:epc>)
endif()
