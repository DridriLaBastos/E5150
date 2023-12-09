project(platform)

add_library(platform)

if (UNIX)
	target_sources(platform PRIVATE platform_unix.c)
elseif(WIN32)
	target_sources(platform PRIVATE platform_win32.c)
else()
	message(FATAL_ERROR "System ${CMAKE_SYSTEM_NAME} not supported")
endif()
