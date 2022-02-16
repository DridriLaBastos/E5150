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

set(CMAKE_XCODE_SCHEME_WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/e5150)
set(CMAKE_XCODE_SCHEME_WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/e5150)

#Maybe using a configuration file later
#configure_file(config.hpp.in config.hpp)

file(GLOB CORE_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/src/*.cpp")

file(GLOB CORE_HEADERS "${PROJECT_SOURCE_DIR}/core/include/*.hpp")
file(GLOB CPU_HEADERS "${PROJECT_SOURCE_DIR}/core/src/cpu/*.cpp")
file(GLOB FLOPPY_HEADERS "${PROJECT_SOURCE_DIR}/core/src/floppy/*.cpp")

set_source_files_properties(${CORE_HEADERS} ${DEBUGGER_HEADERS} ${CPU_HEADERS} ${FLOPPY_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
# source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${DEBUGGER_SRC} ${CORE_SRC} ${DEBUGGER_HEADERS} ${DEBUGGER_HEADERS} main.cpp)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES main.cpp ${CORE_SRC} ${CORE_HEADERS})

# if (ENABLE_DEBUGGER)
# 	add_subdirectory(debugger)

# 	set(Python3_USE_STATIC_LIBS TRUE)
# 	find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
# 	add_compile_definitions(DEBUGGER)
# 	file(GLOB DEBUGGER_SRC CONFIGURE_DEPENDS "debugger/*.cpp")
# 	set(DEBUGGER_INC_DIR "debugger")
# endif()

# Addind the header only files as sources to make theme apparent in IDE, only using source_group isn't enougth if th source file isn't used
add_executable(epc main.cpp ${CORE_SRC} ${CORE_HEADERS})
add_dependencies(epc xedlib asm-test)

target_include_directories(epc PRIVATE ${PROJECT_SOURCE_DIR}/core/include PRIVATE ${PROJECT_SOURCE_DIR}/third-party/spdlog/include PRIVATE ${PROJECT_SOURCE_DIR}/third-party/include PRIVATE ${PROJECT_BINARY_DIR}/third-party/include)
target_link_directories(epc PRIVATE ${PROJECT_BINARY_DIR}/third-party/lib)
target_link_libraries(epc xed)
target_precompile_headers(epc PRIVATE ${PROJECT_SOURCE_DIR}/core/include/pch.hpp)

if (ENABLE_DEBUGGER)
	file(GLOB DEBUGGER_SRC CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/debugger/*.cpp)
	file(GLOB DEBUGGER_HEADERS ${PROJECT_SOURCE_DIR}/debugger/*.hpp)

	set_source_files_properties(${DEBUGGER_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
	source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${DEBUGGER_SRC} ${DEBUGGER_HEARDERS})

	target_sources(epc PRIVATE ${DEBUGGER_SRC} ${DEBUGGER_HEADERS})
	target_include_directories(epc PRIVATE ${CMAKE_SOURCE_DIR})
	add_compile_definitions(DEBUGGER)
endif()

add_custom_target(run ${PROJECT_BINARY_DIR}/epc
					WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
					DEPENDS epc)
