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

set(E5150_PRECOMPILED_HEADER_FILE ${CMAKE_SOURCE_DIR}/common_pch.hpp)

option(USE_DEBUGGER "Compile the integrated debugger" OFF)
option(USE_MOLD "Use mold linker to increase link performances" OFF)

add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)

if (${USE_DEBUGGER})
	add_compile_definitions(DEBUGGER_ON)
endif ()

if (NOT WIN32)
	add_compile_options(-Wno-switch -Wall -Wextra)
else ()
	add_link_options("$<$<CONFIG:RelWithDebInfo>:/PROFILE>")
endif()

include_directories(
		${CMAKE_SOURCE_DIR}
		${CMAKE_SOURCE_DIR}/third-party
		${CMAKE_SOURCE_DIR}/third-party/spdlog/include
		${CMAKE_SOURCE_DIR}/third-party/imgui
		${CMAKE_SOURCE_DIR}/third-party/glfw/include
		${CMAKE_SOURCE_DIR}/third-party/CLI11/include

		${CMAKE_BINARY_DIR}/third-party/include
)

add_subdirectory(third-party)
add_subdirectory(test)
add_subdirectory(gui)
add_subdirectory(platform)
add_subdirectory(core)

if (MOLD_PATH)
	message(WARNING "Using mold is still experimental")
	string(TOLOWER "${CMAKE_C_COMPILER_ID}" COMPILER_ID_LOWERCASE)

	if("${COMPILER_ID_LOWERCASE}" STREQUAL "gnu")
		if(${CMAKE_C_COMPILER_VERSION} VERSION_LESS "12.1.0")
			message(WARNING "Mold required but not supported in this cmake for version of gcc lower than 12.1.0 the current version is ${CMAKE_C_COMPILER_VERSION}. Default linker used")
		else()
			add_link_options(-B${MOLD_PATH})
		endif()
	elseif("${COMPILER_ID_LOWERCASE}" STREQUAL "clang" OR "${COMPILER_ID_LOWERCASE}" STREQUAL "appleclang")
		add_link_options("-fuse-ld=${MOLD_PATH}")
	else()
		message(WARNING "Unsupported compiler id ${COMPILER_ID_LOWERCASE}, default linker used")
	endif()
endif()

#Maybe using a configuration file later
#configure_file(config.hpp.in config.hpp)

# set(EMULATOR_SRC ${CORE_SRC} ${CPU_SRC} ${FLOPPY_SRC})
# set(PLATFORM_SRC platform/platform_unix.c)

# if(WIN32)
# 	set(PLATFORM_SRC platform/platform_win32.c)
# endif()

# find_package(OpenGL REQUIRED)

# add_executable(epc main.cpp ${EMULATOR_SRC} ${PLATFORM_SRC}
# 	${PROJECT_SOURCE_DIR}/gui/gui.cpp
# 	${PROJECT_SOURCE_DIR}/gui/ui_loop.cpp
# )

# target_link_libraries(epc core glfw OpenGL::GL imgui)

# set(HOTRELOAD_COPY_FULL_PATH "$<TARGET_FILE_DIR:hot_reload_draw>/$<TARGET_FILE_PREFIX:hot_reload_draw>$<TARGET_FILE_BASE_NAME:hot_reload_draw>_c$<TARGET_FILE_SUFFIX:hot_reload_draw>")

# target_compile_definitions(epc PRIVATE
# 	DRAW_LIBRARY_FULL_PATH="$<TARGET_FILE:hot_reload_draw>"
# 	DRAW_LIBRARY_COPY_FULL_PATH="${HOTRELOAD_COPY_FULL_PATH}")
# add_dependencies(epc asm-test)

# #TODO: Maybe I want the macro DEBUGGER_ON to be defined inside debugger.hpp ?
# if (ENABLE_DEBUGGER)
# 	target_sources(epc PUBLIC debugger/debugger.cpp debugger/cli.cpp debugger/commands.cpp)
# 	target_compile_definitions(epc PRIVATE DEBUGGER_ON)
# 	target_compile_definitions(hot_reload_draw PRIVATE DEBUGGER_ON)
# endif()

# add_custom_target(run ${PROJECT_BINARY_DIR}/epc
# 					WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
# 					DEPENDS epc)

# if(WIN32)
# 	add_custom_command(TARGET epc POST_BUILD
# 			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:imgui> $<TARGET_FILE_DIR:epc>)
# endif()


add_executable(epc main.cpp)
target_link_libraries(epc gui core)

if (${WIN32})
	string(TIMESTAMP CURRENT_TIMESTAMP "%H_%m_%s")
	set_target_properties(epc PROPERTIES PDB_NAME "epc_${CURRENT_TIMESTAMP}.pdb")
endif()
add_custom_target(run $<TARGET_FILE:epc> WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
