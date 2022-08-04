project(thirdparty)

set(XED_LIB_NAME "libxed.a")
set(XED_LTO_FLAG "-flto")

if (WIN32)
	set (XED_LIB_NAME "xed.lib")
	set(XED_LTO_FLAG "/GL")
endif()

if((NOT EXISTS ${PROJECT_BINARY_DIR}/lib/${XED_LIB_NAME}) OR (NOT EXISTS ${PROJECT_BINARY_DIR}/include/xed))
	message("COMPILING XED")
	include(ProcessorCount)

	find_package(Python3 COMPONENTS Interpreter REQUIRED)

	ProcessorCount(CPU_COUNT)

	message("Found ${CPU_COUNT} cpus for building xed")

	# xed is suvh a PAIN to build...
	# This NEEDS to be extensivly tested on multiple platforms
	# Why sometimes the command works and sometimes not ?
	set(XED_BUILD_INSTALL_FLAGS --build-dir=${PROJECT_BINARY_DIR}/obj --install-dir=${PROJECT_BINARY_DIR}/kits --prefix=${PROJECT_BINARY_DIR})

	if (WIN32)
		set(XED_BUILD_INSTALL_FLAGS "--install-dir=${PROJECT_BINARY_DIR}")
	endif()

	execute_process(COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/xed/xed/mfile.py --jobs=${CPU_COUNT} --static --extra-flags=${XED_LTO_FLAG} --opt=3 ${XED_BUILD_INSTALL_FLAGS} --no-amd --no-via --no-encoder install
		COMMAND_ECHO STDOUT)

	find_library(XED_LIB_PATH ${XED_LIB_NAME}
			PATHS ${PROJECT_BINARY_DIR}/lib
			DOC "The xed decoding library used to decode instructions"
			REQUIRED)

	if(XED_LIB_PATH)
		message("XED LIBRARY SUCCESSFULLY BUILDED: (found at '${XED_LIB_PATH}')")
	endif()
endif()
