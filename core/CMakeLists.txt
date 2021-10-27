project(libcore)

message("Build type: '${CMAKE_BUILD_TYPE}'")

add_compile_definitions($<$<CONFIG:Debug>:DEBUG_BUILD>)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_custom_command(OUTPUT third-party/lib/libxed.a third-party/lib/libxed-ild.a third-party/include/xed
					COMMAND ${CMAKE_COMMAND} -E make_directory third-party/xed/build
					COMMAND ${PYTHON3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/third-party/xed/xed/mfile.py --jobs=4 --build-dir=third-party/xed/build/obj --src-dir=${CMAKE_CURRENT_SOURCE_DIR}/third-party/xed/xed --static --opt=3 --prefix=third-party --install-dir=third-party/xed/kits/xed-install-date-os-cpu --no-amd --no-via --no-encoder --extra-flags=-flto install
					)

add_custom_target(xed DEPENDS third-party/lib/libxed.a)

add_library(core STATIC src/arch.cpp src/cpu.cpp src/floppy.cpp src/pic.cpp src/pit.cpp src/ports.cpp src/ppi.cpp src/ram.cpp)

add_dependencies(core xed)
target_include_directories(core PUBLIC ${PROJECT_BINARY_DIR}/third-party/include PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_precompile_headers(core PUBLIC include/pch.hpp)
