project("E5150 gui")
cmake_minimum_required(VERSION 3.15)

add_library(gui STATIC common/imgui_common.cpp)
add_library(hot_reload_draw MODULE common/hot_reload_draw.cpp)

target_compile_definitions(gui PRIVATE DRAW_LIBRARY_FULL_PATH="$<TARGET_FILE:hot_reload_draw>"
		DRAW_LIBRARY_PREFIX_BASE_NAME="$<TARGET_FILE_PREFIX:hot_reload_draw>$<TARGET_FILE_BASE_NAME:hot_reload_draw>"
		DRAW_LIBRARY_FILE_EXTENSION="$<TARGET_FILE_SUFFIX:hot_reload_draw>")

target_link_libraries(hot_reload_draw imgui)

add_dependencies(gui hot_reload_draw)

#TODO: adding precompiled header

if(APPLE)
	file(GLOB GUI_PLATFORM_SRC CONFIGURE_DEPENDS macos/*.mm macos/*.cpp)
elseif(WIN32)
	file(GLOB GUI_PLATFORM_SRC CONFIGURE_DEPENDS win/gui_d3d11.cpp)
	set(DOLLAR_LITERAL "$")
	set(OPEN_CURLY_BRACKET_LITERAL "(")
	set(CLOSE_CURLY_BRACKET_LITERAL ")")
	set(TIMESTAMP_PDB "/PDB:hot_reload_draw_${DOLLAR_LITERAL}${OPEN_CURLY_BRACKET_LITERAL} get-date -uformat %H%M%S${CLOSE_CURLY_BRACKET_LITERAL}.pdb")
	message(${TIMESTAMP_PDB})
	target_link_options(hot_reload_draw PRIVATE ${TIMESTAMP_PDB})
else()
	message(FATAL_ERROR "Only MacOS and Windows are supported for the moment")
endif()

target_sources(gui PRIVATE ${GUI_PLATFORM_SRC})
