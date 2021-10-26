project(e5150)

enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS ${CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS} S s)
set(CMAKE_ASM_NASM_FLAGS ${CMAKE_ASM_NASM_FLAGS} -f bi -I${PROJECT_SOURCE_DIR})

message("${CMAKE_ASM_NASM_FLAGS}")
message("${CMAKE_CXX_FLAGS}")


#message("${CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS}")

add_executable(epc.out main.cpp)
add_library(asm OBJECT test/bios.s test/interrupts.s test/jmp.s)
#add_dependencies(epc.out bios interrupts jmp)

file(DOWNLOAD https://ibm.retropc.se/bios/BIOS_5150_24APR81_U33.BIN ${PROJECT_BINARY_DIR}/test/ibm_bios.bin SHOW_PROGRESS)

#add_custom_target(bios ${CMAKE_ASM_NASM_COMPILER} -f bin -I${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/test/bios.s -o ${PROJECT_BINARY_DIR}/test/bios.bin)
#add_custom_target(interrupts ${CMAKE_ASM_NASM_COMPILER} -f bin -I${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/test/interrupts.s -o ${PROJECT_BINARY_DIR}/test/interrupts.bin)
#add_custom_target(jmp ${CMAKE_ASM_NASM_COMPILER} -f bin -I${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/test/jmp.s -o ${PROJECT_BINARY_DIR}/test/jmp.bin)

add_dependencies(epc.out asm)

target_link_directories(epc.out PUBLIC "${PROJECT_BINARY_DIR}/../core/third-party/lib")

target_link_libraries(epc.out PUBLIC core PRIVATE xed)
