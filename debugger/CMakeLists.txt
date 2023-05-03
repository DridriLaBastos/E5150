add_library(decom SHARED communication.c)

file(GLOB DEBUGGER_C_SRC "")
file(GLOB DEBUGGER_CPP_SRC *.cpp)
set(DEBUGGER_SRC ${DEBUGGER_C_SRC} ${DEBUGGER_CPP_SRC} PARENT_SCOPE)

add_subdirectory(server)

set(DEBUGGER_SERVER_EXE $<TARGET_FILE:debugger-server> PARENT_SCOPE)
