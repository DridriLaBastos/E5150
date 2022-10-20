project("E5150 gui")

add_library(gui STATIC)
set(GUI_LIB_ALL_SRC "")

if(APPLE)
	file(GLOB GUI_LIB_ALL_SRC CONFIGURE_DEPENDS macos/*.mm macos/*.cpp)
else()
	message(FATAL_ERROR "Only OSX gui supported for now")
endif()

target_sources(gui PRIVATE ${GUI_LIB_ALL_SRC})
