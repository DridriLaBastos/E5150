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

add_subdirectory(core)
add_subdirectory(e5150)

# Only setting the flags doesn't enable the LTO options inside the IDE (at least in xcode) (so in the options of the IDE the only way to know that LTO are unable is by looking at the linker flags which is not very intuitive) but they enable monolithic LTO.
#The way I am doing it now by setting a cmake option make the corresponding LTO options to unable inside the IDE but (at least for xcode) thin LTO are unabled and not monolithic
#I don't really understanf the differences between the two, for what I read monolithyc might be slightly longer and better but no differences found when benchmarking

#if (CMAKE_HOST_UNIX)
#add_compile_options($<$<CONFIG:RELEASE>:-flto>)
#add_link_options   ($<$<CONFIG:RELEASE>:-flto>)
#endif()
set_property(DIRECTORY core PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set_property(DIRECTORY e5150 PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

add_custom_target(run
					COMMAND epc.out
					WORKING_DIRECTORY e5150
				)
