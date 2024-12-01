project("E5150 gui" DESCRIPTION "Gui code of the E5150 emulator")

add_library(hot_reload_draw MODULE hot_reload_draw.cpp)
add_library(gui gui.cpp ui_loop.cpp)

target_precompile_headers(gui PRIVATE ${E5150_PRECOMPILED_HEADER_FILE})
target_precompile_headers(hot_reload_draw PRIVATE ${E5150_PRECOMPILED_HEADER_FILE})

target_compile_options(hot_reload_draw PRIVATE)

target_link_libraries(gui PRIVATE imgui glfw platform)
target_link_libraries(hot_reload_draw PRIVATE imgui $<$<BOOL:${USE_DEBUGGER}>:xed>)

add_dependencies(gui hot_reload_draw)

set(HOTRELOAD_COPY_FULL_PATH "$<TARGET_FILE_DIR:hot_reload_draw>/$<TARGET_FILE_PREFIX:hot_reload_draw>$<TARGET_FILE_BASE_NAME:hot_reload_draw>_c$<TARGET_FILE_SUFFIX:hot_reload_draw>")
set(IBM_BIOS_PATH ${PROJECT_BINARY_DIR}/ibm_bios.bin)
target_compile_definitions(gui PRIVATE
	DRAW_LIBRARY_FULL_PATH="$<TARGET_FILE:hot_reload_draw>"
	DRAW_LIBRARY_COPY_FULL_PATH="${HOTRELOAD_COPY_FULL_PATH}")

file(DOWNLOAD http://www.minuszerodegrees.net/bios/BIOS_IBM5150_24APR81_5700051_U33.BIN ${IBM_BIOS_PATH} 
EXPECTED_HASH SHA256=a259306c47b43e14ef6378d7bdeff67331a2519d363bc36b3f354bc91a6874c8
SHOW_PROGRESS)

target_compile_definitions(gui PRIVATE IBM_BIOS_PATH="${IBM_BIOS_PATH}")
