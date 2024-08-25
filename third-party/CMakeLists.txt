cmake_minimum_required(VERSION 3.17 FATAL_ERROR) # Submodules recurse in FetchContent_Declare

project(thirdparty)

include(ProcessorCount)
ProcessorCount(CPU_COUNT)
message(VERBOSE "Found ${CPU_COUNT} cpus for building xed")

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
		set(BUILD_MOLD TRUE)

		set(MOLD_GIT_REPO https://github.com/rui314/mold)
		set(MOLD_BRANCH "v${MOLD_VERSION}")

		if(APPLE)
			set(MOLD_GIT_REPO https://github.com/bluewhalesystems/sold)
			set(MOLD_BRANCH "main")
			set(XED_EXTRA_FLAGS "${XED_EXTRA_FLAGS} -fuse-ld=${PROJECT_BINARY_DIR}/mold/ld64")
		endif()

		if(NOT EXISTS ${PROJECT_SOURCE_DIR}/mold)
			execute_process(COMMAND git clone --recurse-submodules -b ${MOLD_BRANCH} ${MOLD_GIT_REPO} ${PROJECT_SOURCE_DIR}/mold)
		endif()

		if(NOT EXISTS ${PROJECT_BINARY_DIR}/mold)
			message(STATUS "Configuring mold")
			execute_process(COMMAND ${CMAKE_COMMAND} -S ${PROJECT_SOURCE_DIR}/mold -B ${PROJECT_BINARY_DIR}/mold -DCMAKE_BUILD_TYPE=Release -DMOLD_LTO=ON)
		endif()
	endif()

	if(BUILD_MOLD)
		message(STATUS "Compilation of mold")
		execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/mold -j ${CPU_COUNT}
			COMMAND_ECHO STDOUT
			RESULT_VARIABLE MOLD_COMPILE_SUCCESS
		)
		set(MOLD_PATH "${PROJECT_BINARY_DIR}/mold/ld64" PARENT_SCOPE)
	endif()
endif()

set(BUILD_SHARED_LIBS ON)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(glfw)

set(CLI11_WARNINGS_AS_ERRORS OFF)
set(CLI11_SINGLE_FILE OFF)
set(CLI11_PRECOMPILED ON)
add_subdirectory(CLI11)

find_package(OpenGL REQUIRED)

add_library(imgui SHARED
	${CMAKE_SOURCE_DIR}/third-party/imgui/imgui_draw.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/imgui_tables.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/imgui_widgets.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/imgui_demo.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/imgui.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/backends/imgui_impl_glfw.cpp
	${CMAKE_SOURCE_DIR}/third-party/imgui/backends/imgui_impl_opengl3.cpp
)
target_link_libraries(imgui glfw OpenGL::GL)

set(XED_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}xed${CMAKE_STATIC_LIBRARY_SUFFIX})

if (${USE_DEBUGGER})
	set(XED_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}xed${CMAKE_SHARED_LIBRARY_SUFFIX})	
endif()

set(XED_EXTRA_FLAGS "-flto")
if (WIN32)
	set(XED_EXTRA_FLAGS "/GL")
	target_compile_definitions(imgui PUBLIC "IMGUI_API=__declspec(dllexport)")
else ()
	target_compile_definitions(imgui PUBLIC "IMGUI_API=__attribute__((visibility(\"default\")))")
endif()

if((NOT EXISTS ${PROJECT_BINARY_DIR}/lib/${XED_LIB_NAME}) OR (NOT EXISTS ${PROJECT_BINARY_DIR}/include/xed))
	message(STATUS "COMPILING XED")
	
	find_package(Python3 COMPONENTS Interpreter REQUIRED)

	# There must be some static variable in libxed or something because when compiled as a static library, when
	# linked with the shared library hot_reload_draw I have undefined symbols inside xed
	if(${USE_DEBUGGER})
		set(XED_LIB_TYPE_FLAG --shared)
	else()
		set(XED_LIB_TYPE_FLAG --static)
	endif()

	execute_process(COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/xed/xed/mfile.py" --jobs=${CPU_COUNT} --build-dir=${PROJECT_BINARY_DIR}/obj --install-dir=${PROJECT_BINARY_DIR}/kits --prefix=${PROJECT_BINARY_DIR} --extra-flags=${XED_EXTRA_FLAGS} --opt=3 ${XED_LIB_TYPE_FLAG} --no-amd --no-via --no-encoder install)

	if(${WIN32})
		file(COPY ${PROJECT_BINARY_DIR}/kits/lib DESTINATION ${PROJECT_BINARY_DIR})
		file(COPY ${PROJECT_BINARY_DIR}/kits/include/xed DESTINATION  ${PROJECT_BINARY_DIR}/include)
		if(${USE_DEBUGGER})
			file(COPY ${PROJECT_BINARY_DIR}/kits/bin/xed.dll ${PROJECT_BINARY_DIR}/kits/bin/xed-ild.dll DESTINATION ${PROJECT_BINARY_DIR}/lib)
		endif()
	endif ()
endif()

add_library(xed SHARED IMPORTED GLOBAL)

set_target_properties(xed PROPERTIES IMPORTED_IMPLIB ${PROJECT_BINARY_DIR}/lib/${XED_LIB_NAME})
