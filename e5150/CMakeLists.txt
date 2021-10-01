add_subdirectory(../core)

add_executable(e5150 main.cpp)

target_link_libraries(core PUBLIC e5150)
target_include_directories(e5150 PUBLIC "${PROJECT_BINATY_DIR}/core/include")

add_library(core)