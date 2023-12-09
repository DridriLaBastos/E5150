project(e5150-debugger)

file(GLOB DEBUGGER_SRC CONFIGURE_DEPENDS *.cpp *.hpp)

add_library(debugger STATIC ${DEBUGGER_SRC})

target_precompile_headers(debugger PUBLIC ${COMMON_PRECOMPILED_HEADER})
