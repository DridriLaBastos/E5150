project(libcore)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_library(core STATIC src/arch.cpp src/cpu.cpp src/floppy.cpp src/pic.cpp src/pit.cpp src/ports.cpp src/ppi.cpp src/ram.cpp third-party/lib/libxed.a)

add_custom_command(OUTPUT third-party/lib/libxed.a
					COMMAND ${CMAKE_COMMAND} -E make_directory third-party/xed/build
					COMMAND ${PYTHON3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/third-party/xed/xed/mfile.py --jobs=4 --build-dir=third-party/xed/build/obj --src-dir=${CMAKE_CURRENT_SOURCE_DIR}/third-party/xed/xed --static --opt=3 --prefix=third-party --install-dir=third-party/xed/kits/xed-install-date-os-cpu --no-amd --no-via --no-encoder --compress-operands install
					)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third-party/include PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
target_precompile_headers(core PUBLIC include/pch.hpp)