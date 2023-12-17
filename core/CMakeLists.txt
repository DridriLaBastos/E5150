project(E5150_CORE_LIB)

file(GLOB_RECURSE CORE_SRC CONFIGURE_DEPENDS src/*.cpp *.hpp)

add_library(core STATIC ${CORE_SRC})

if (${USE_DEBUGGER})
	file(GLOB DEBUGGER_SRC CONFIGURE_DEPENDS debugger/src/*.cpp debugger/*.hpp)
	target_sources(core PRIVATE ${DEBUGGER_SRC})
	target_include_directories(core PRIVATE ${PROJECT_SOURCE_DIR})
endif ()

target_link_libraries(core PRIVATE xed)

set(LOCAL_PRECOMPILED_HEADER_PATH ${PROJECT_SOURCE_DIR}/pch.hpp)

target_precompile_headers(core	PUBLIC $<$<COMPILE_LANGUAGE:CXX,OBJC,OBJCXX>:${COMMON_PRECOMPILED_HEADER}>
								PRIVATE $<$<COMPILE_LANGUAGE:CXX,OBJC,OBJCXX>:${LOCAL_PRECOMPILED_HEADER_PATH}>)
