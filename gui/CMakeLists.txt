project("E5150 gui" DESCRIPTION "Hot reload part of the E5150 gui")

add_library(hot_reload_draw MODULE common/hot_reload_draw.cpp)

target_link_libraries(hot_reload_draw imgui)
