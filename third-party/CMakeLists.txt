project(thirdparty)

set(XED_LIB_NAME "libxed.a")
set(XED_LTO_FLAG "-flto")

if (WIN32)
	set (XED_LIB_NAME "xed.lib")
	set(XED_LTO_FLAG "/GL")
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

file(GLOB IMGUI_COMMON_SRC CONFIGURE_DEPENDS imgui/*.cpp)

add_library(imgui SHARED ${IMGUI_COMMON_SRC})

if(APPLE)
	target_sources(imgui
			PRIVATE imgui/backends/imgui_impl_osx.mm imgui/backends/imgui_impl_metal.mm)
	find_library(APP_KIT NAMES AppKit REQUIRED)
	find_library(METAL_KIT NAMES MetalKit REQUIRED)
	find_library(METAL NAMES Metal REQUIRED)
	find_library(QUARTZ_CORE QuartzCore REQUIRED)
	find_library(GAME_CONTROLLER GameController REQUIRED)
	target_link_libraries(imgui ${APP_KIT} ${METAL_KIT} ${METAL} ${QUARTZ_CORE} ${GAME_CONTROLLER})
elseif(WIN32)
	target_sources(imgui PRIVATE imgui/backends/imgui_impl_dx11.cpp imgui/backends/imgui_impl_win32.cpp)
	target_link_libraries(imgui d3d11.lib d3dcompiler.lib dxgi.lib)
	if (MSVC)
		target_compile_definitions(imgui PRIVATE "IMGUI_API=__declspec(dllexport)")
	endif()
else()
	message(FATAL_ERROR "System ${CMAKE_SYSTEM_NAME} not supported yet")
endif()
