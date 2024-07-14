project(E5150_CORE_LIB)

file(GLOB_RECURSE CORE_SRC CONFIGURE_DEPENDS src/*.cpp *.hpp)
file(GLOB_RECURSE CORE_HDR *.hpp)

add_library(core STATIC ${CORE_HDR}
				src/arch.cpp
				src/cpu/8086.cpp)

if (${USE_DEBUGGER})
	file(GLOB DEBUGGER_SRC CONFIGURE_DEPENDS debugger/src/*.cpp debugger/*.hpp)
	target_sources(core PRIVATE ${DEBUGGER_SRC})
	target_include_directories(core PRIVATE ${PROJECT_SOURCE_DIR})
endif ()

target_link_libraries(core PRIVATE xed)

target_precompile_headers(core	PUBLIC $<$<COMPILE_LANGUAGE:CXX,OBJC,OBJCXX>:${E5150_PRECOMPILED_HEADER_FILE}>)
