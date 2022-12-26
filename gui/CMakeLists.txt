project("E5150 gui")

add_library(gui STATIC common/imgui_common.cpp)
add_library(hot_reload_draw MODULE common/hot_reload_draw.cpp)

target_compile_definitions(gui PRIVATE HOT_RELOAD_DRAW_PATH="$<TARGET_FILE:hot_reload_draw>")

target_link_libraries(hot_reload_draw imgui)

add_dependencies(gui hot_reload_draw)

#TODO: adding precompiled header

if(APPLE)
	file(GLOB GUI_PLATFORM_SRC CONFIGURE_DEPENDS macos/*.mm macos/*.cpp)
elseif(WIN32)
	file(GLOB GUI_PLATFORM_SRC CONFIGURE_DEPENDS win/gui_d3d11.cpp)
else()
	message(FATAL_ERROR "Only MacOS and Windows are supported for the moment")
endif()

target_sources(gui PRIVATE ${GUI_PLATFORM_SRC})
