cmake_minimum_required(VERSION 3.10)

#setting project infos
project(E5150 VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
configure_file("${PROJECT_SOURCE_DIR}/include/config.in" "${PROJECT_SOURCE_DIR}/include/config.hpp")

set(CPPSRC main.cpp src/ram.cpp src/ppi.cpp src/ports.cpp src/pit.cpp src/pic.cpp src/floppy.cpp src/fdc.cpp src/cpu.cpp src/arch.cpp)

find_package(SFML 2.0 COMPONENTS system)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/third-party/spdlog/include ${SFML_INCLUDE_PATH})
link_libraries(xed sfml-system)

#add the executable
add_executable(epc.out ${CPPSRC})