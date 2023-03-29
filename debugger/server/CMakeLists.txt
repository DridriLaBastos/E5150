cmake_minimum_required(VERSION 3.14)

project(debugger-server VERSION 0.0.1
        DESCRIPTION "Embeded debugger of the E5150 emulator")

include(FetchContent)

FETCHCONTENT_DECLARE(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
        GIT_TAG v2.9
        GIT_PROGRESS TRUE
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/third-party/argparse
)
FetchContent_MakeAvailable(argparse)

add_executable(debugger-server main.cpp)
target_include_directories(debugger-server PRIVATE ${argparse_SOURCE_DIR}/include)
target_compile_definitions(debugger-server PRIVATE DEBUGGER_VERSION="${PROJECT_VERSION}")

set(DEBUGGER_SERVER_EXE $<TARGET_FILE:debugger-server> PARENT_SCOPE)
