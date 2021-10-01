add_executable(e5150 main.cpp)

add_dependencies(e5150 core)

target_link_libraries(e5150 PUBLIC core)
target_include_directories(e5150 PUBLIC "${PROJECT_SOURCE_DIR}")
