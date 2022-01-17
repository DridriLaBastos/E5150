project(libcore)

if(TEST)
	add_compile_definitions(UNIT_TEST)
endif(TEST)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_custom_command(OUTPUT third-party/lib/libxed.a third-party/lib/libxed-ild.a third-party/include/xed
					COMMAND ${CMAKE_COMMAND} -E make_directory third-party/xed/build
					COMMAND ${PYTHON3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/third-party/xed/xed/mfile.py --jobs=4 --build-dir=third-party/xed/build/obj --src-dir=${CMAKE_CURRENT_SOURCE_DIR}/third-party/xed/xed --static --opt=3 --prefix=third-party --install-dir=third-party/xed/kits/xed-install-date-os-cpu --no-amd --no-via --no-encoder --extra-flags=-flto install
					)

add_custom_target(xed DEPENDS third-party/lib/libxed.a)

file(GLOB COMPILED_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB DEBUGGER_SOURCES CONFIGURE_DEPENDS "debugger/*.cpp")
file(GLOB INCLUDED_SOURCES CONFIGURE_DEPENDS "src/cpu/*.cpp" "src/floppy/*.cpp")
file(GLOB HEADERS CONFIGURE_DEPENDS "include/*.hpp" "debugger/*.hpp")

set_source_files_properties(${INCLUDED_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group(TREE ${PROJECT_SOURCE_DIR}/src FILES ${COMPILED_SOURCES} ${INCLUDED_SOURCES})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${HEADERS})

add_library(core STATIC ${COMPILED_SOURCES} ${DEBUGGER_SOURCES} ${DEBUGGER_SOURCES} ${INCLUDED_SOURCES} ${HEADERS})

if(CMAKE_AR)
	add_custom_command(TARGET core POST_BUILD
					COMMAND ${CMAKE_AR} ARGS -x third-party/lib/libxed.a
					COMMAND ${CMAKE_AR} ARGS -rvu $<TARGET_FILE:core> *.o
					WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
					)
endif(CMAKE_AR)

add_dependencies(core xed)
target_include_directories(core PUBLIC ${PROJECT_BINARY_DIR}/third-party/include PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_precompile_headers(core PUBLIC include/pch.hpp)
