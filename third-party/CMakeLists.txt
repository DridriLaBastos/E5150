project(thirdparty)

include(ProcessorCount)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

ProcessorCount(CPU_COUNT)

message("Found ${CPU_COUNT} cpus for building xed")

set(XED_LIB "libxed.a")

if (MSVC)
	set (XED_LIB "libxed.lib")
endif()

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/lib/${XED_LIB} ${PROJECT_BINARY_DIR}/include/xed
					COMMAND ${PYTHON3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/xed/xed/mfile.py --jobs=${CPU_COUNT} --static --extra-flags="-flto=thin" --opt=3 --prefix=${PROJECT_BINARY_DIR} --no-amd --no-via --no-encoder install
				)

add_custom_target(xedlib DEPENDS ${PROJECT_BINARY_DIR}/lib/${XED_LIB})
