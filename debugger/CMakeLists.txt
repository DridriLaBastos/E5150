project(debugger)

find_package(Python3 COMPONENTS Interpreter REQUIRED)


add_subdirectory(communication)

add_library(debugger OBJECT debugger.cpp)

target_compile_definitions(debugger PRIVATE PYTHON3_EXECUTABLE_PATH="${Python3_EXECUTABLE}")
if(WIN32)
	target_sources(debugger PRIVATE platform/platform_win.c)
endif()