cmake_minimum_required(VERSION 3.17 FATAL_ERROR) # Submodules recurse in FetchContent_Declare

project(thirdparty)

set(XED_LIB_NAME "libxed.a")
set(XED_LTO_FLAG "-flto")

set(BUILD_SHARED_LIBS ON)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(glfw)

set(CLI11_WARNINGS_AS_ERRORS OFF)
set(CLI11_SINGLE_FILE ON)
set(CLI11_PRECOMPILED OFF)
add_subdirectory(CLI11)

add_library(imgui SHARED
	${CMAKE_SOURCE_DIR}/third-party/imgui/imgui_draw.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/imgui_tables.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/imgui_widgets.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/imgui_demo.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/imgui.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/backends/imgui_impl_glfw.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/backends/imgui_impl_opengl3.cpp
)
target_link_libraries(imgui glfw)

if (WIN32)
	set (XED_LIB_NAME "xed.lib")
	set(XED_LTO_FLAG "/GL")
	target_compile_definitions(imgui PUBLIC "IMGUI_API=__declspec(dllexport)")
else ()
	target_compile_definitions(imgui PUBLIC "IMGUI_API=__attribute__((visibility(\"default\")))")
endif()

if((NOT EXISTS ${PROJECT_BINARY_DIR}/lib/${XED_LIB_NAME}) OR (NOT EXISTS ${PROJECT_BINARY_DIR}/include/xed))
	message(STATUS "COMPILING XED")
	include(ProcessorCount)

	find_package(Python3 COMPONENTS Interpreter REQUIRED)

	ProcessorCount(CPU_COUNT)

	message(VERBOSE "Found ${CPU_COUNT} cpus for building xed")

	# xed is such a PAIN to build...
	# This NEEDS to be extensively tested on multiple platforms
	# Why sometimes the command works and sometimes not ?
	set(XED_BUILD_INSTALL_FLAGS --build-dir=${PROJECT_BINARY_DIR}/obj --install-dir=${PROJECT_BINARY_DIR}/kits --prefix=${PROJECT_BINARY_DIR})

	if (WIN32)
		set(XED_BUILD_INSTALL_FLAGS "--install-dir=${PROJECT_BINARY_DIR}")
	endif()

	execute_process(COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/xed/xed/mfile.py --jobs=${CPU_COUNT} --static --extra-flags=${XED_LTO_FLAG} --opt=3 ${XED_BUILD_INSTALL_FLAGS} --no-amd --no-via --no-encoder install
		COMMAND_ECHO STDOUT)
endif()

find_library(XED_LIB_PATH ${XED_LIB_NAME}
		PATHS ${PROJECT_BINARY_DIR}/lib
		DOC "path to the xed library"
		REQUIRED)

if (XED_LIB_PATH-NOT_FOUND)
	message(ERROR "Unable to build xed")
else()
	message(DEBUG "xed lib built at ${XED_LIB_PATH}")
endif()

if (USE_MOLD)
	set(MOLD_VERSION "1.11.0")
	include(FetchContent)

	# TODO: Is it possible to conveniently put this on a toolchain file ?
	# On Linux platform there is binary version already usable for mold
	if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		FetchContent_Declare(mold
			URL "https://github.com/rui314/mold/releases/download/v${MOLD_VERSION}/mold-${MOLD_VERSION}-x86_64-linux.tar.gz"
			SOURCE_DIR ${PROJECT_BINARY_DIR}/mold
		)

		set(MOLD_PATH "${PROJECT_BINARY_DIR}/mold/bin/mold" PARENT_SCOPE)
	else()
		FetchContent_Declare(mold
			SOURCE_DIR ${PROJECT_SOURCE_DIR}/mold
			GIT_REPOSITORY https://github.com/rui314/mold
			GIT_TAG v${MOLD_VERSION}

			GIT_SUBMODULES_RECURSE TRUE
			GIT_PROGRESS TRUE
		)
		set(BUILD_MOLD TRUE)
	endif()
	
	set(MOLD_LTO ON)
	FetchContent_MakeAvailable(mold)

	if(BUILD_MOLD)
		execute_process(COMMAND ${CMAKE_COMMAND} -S ${PROJECT_SOURCE_DIR}/mold -B ${PROJECT_BINARY_DIR}/mold
			COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/mold -j ${CPU_COUNT}
			RESULT_VARIABLE MOLD_COMPILE_SUCCESS
		)
		set(MOLD_PATH $<TARGET_FILE:mold> PARENT_SCOPE)
	endif()
endif()

