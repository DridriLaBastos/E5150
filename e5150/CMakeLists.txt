project(e5150)

enable_language(ASM_NASM)

set(CMAKE_ASM_NASM_OBJECT_FORMAT bin)
set(CMAKE_ASM_NASM_FLAGS "-I ${PROJECT_SOURCE_DIR}")

file(DOWNLOAD https://ibm.retropc.se/bios/BIOS_5150_24APR81_U33.BIN ${PROJECT_BINARY_DIR}/test/ibm_bios.bin SHOW_PROGRESS)

add_custom_command(OUTPUT test/bios.bin
					COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/test/bios.s -o test/bios.bin
					DEPENDS test/bios.s
					BYPRODUCTS test/bios.bin
				)

add_custom_command(OUTPUT test/interrupts.bin
					COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/test/interrupts.s -o test/interrupts.bin
					DEPENDS test/interrupts.s
					BYPRODUCTS test/interrupts.bin
				)

add_custom_command(OUTPUT test/jmp.bin
					COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/test/jmp.s -o test/jmp.bin
					DEPENDS test/jmp.s
					BYPRODUCTS test/jmp.bin
				)

add_custom_command(OUTPUT test/jmp0.bin
				COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/test/jmp0.s -o test/jmp0.bin
				DEPENDS test/jmp0.s
				BYPRODUCTS test/jmp0.bin
			)

add_custom_target(bios DEPENDS test/bios.bin)
add_custom_target(interrupts DEPENDS test/interrupts.bin)
add_custom_target(jmp DEPENDS test/jmp.bin)
add_custom_target(jmp0 DEPENDS test/jmp0.bin)

file(GLOB TEST_ASM_SRC "test/*.s")
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${TEST_ASM_SRC})

add_executable(epc.out main.cpp ${TEST_ASM_SRC})
add_dependencies(epc.out bios interrupts jmp jmp0)

target_link_directories(epc.out PUBLIC "${PROJECT_BINARY_DIR}/../core/third-party/lib")

target_link_libraries(epc.out PUBLIC core)
