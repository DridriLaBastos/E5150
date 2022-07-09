project(thirdparty)

set(XED_LIB_NAME "libxed.a")

if (MSVC)
	set (XED_LIB_NAME "xed.lib")
endif()

if((NOT EXISTS ${PROJECT_BINARY_DIR}/lib/${XED_LIB_NAME}) OR (NOT EXISTS ${PROJECT_BINARY_DIR}/include/xed))
	message("COMPILING XED")
	include(ProcessorCount)

	find_package(Python3 COMPONENTS Interpreter REQUIRED)

	ProcessorCount(CPU_COUNT)

	message("Found ${CPU_COUNT} cpus for building xed")

	execute_process(COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/xed/xed/mfile.py --jobs=${CPU_COUNT} --static --extra-flags="-flto" --opt=3 --verbose=3 --install-dir=${PROJECT_BINARY_DIR} --no-amd --no-via --no-encoder install)

	find_library(XED_LIB_PATH ${XED_LIB_NAME}
			PATHS ${PROJECT_BINARY_DIR}/lib
			DOC "The xed decoding library used to decode instructions"
			REQUIRED)

	if(XED_LIB_PATH)
		message("XED LIBRARY SUCCESSFULLY BUILDED: (found at '${XED_LIB_PATH}')")
	endif()
endif()

# add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/lib/${XED_LIB} ${PROJECT_BINARY_DIR}/include/xed
#					COMMAND ${PYTHON3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/xed/xed/mfile.py --jobs=${CPU_COUNT} --static --extra-flags="-flto=thin" --opt=3 --prefix=${PROJECT_BINARY_DIR} --no-amd --no-via --no-encoder install
#				)

# add_custom_target(xedlib DEPENDS ${PROJECT_BINARY_DIR}/lib/${XED_LIB})
