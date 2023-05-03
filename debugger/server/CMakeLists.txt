cmake_minimum_required(VERSION 3.14)

project(debugger-server VERSION 0.0.1
        DESCRIPTION "Embeded debugger of the E5150 emulator")

add_executable(debugger-server main.cpp)
target_compile_definitions(debugger-server PRIVATE DEBUGGER_VERSION="${PROJECT_VERSION}")
target_link_libraries(debugger-server decom)
