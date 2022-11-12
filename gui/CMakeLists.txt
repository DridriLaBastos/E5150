project("E5150 gui")

file(GLOB GUI_COMMON_SRC CONFIGURE_DEPENDS common/*.cpp)

add_library(gui STATIC)

#TODO: adding precompiled header

if(APPLE)
	file(GLOB GUI_PLATFORM_SRC CONFIGURE_DEPENDS macos/*.mm macos/*.cpp)
elseif(WIN32)
	file(GLOB GUI_PLATFORM_SRC CONFIGURE_DEPENDS win/*.cpp)
else()
	message(FATAL_ERROR "Only MacOS and Windows are supported for the moment")
endif()

target_sources(gui PRIVATE ${GUI_COMMON_SRC} ${GUI_PLATFORM_SRC})
