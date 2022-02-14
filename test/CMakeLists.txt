project(test)

enable_language(ASM_NASM)

set(CMAKE_ASM_NASM_OBJECT_FORMAT bin)
set(CMAKE_ASM_NASM_FLAGS "-I ${CMAKE_SOURCE_DIR}")

file(DOWNLOAD https://ibm.retropc.se/bios/BIOS_5150_24APR81_U33.BIN ${PROJECT_BINARY_DIR}/ibm_bios.bin SHOW_PROGRESS)

add_custom_command(OUTPUT bios.bin
					COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/bios.s -o bios.bin
					DEPENDS ${PROJECT_SOURCE_DIR}/bios.s
					BYPRODUCTS bios.bin
				)

add_custom_command(OUTPUT interrupts.bin
					COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/interrupts.s -o interrupts.bin
					DEPENDS ${PROJECT_SOURCE_DIR}/interrupts.s
					BYPRODUCTS interrupts.bin
				)

add_custom_command(OUTPUT jmp.bin
					COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/jmp.s -o jmp.bin
					DEPENDS ${PROJECT_SOURCE_DIR}/jmp.s
					BYPRODUCTS jmp.bin
				)

add_custom_command(OUTPUT jmp0.bin
				COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/jmp0.s -o jmp0.bin
				DEPENDS ${PROJECT_SOURCE_DIR}/jmp0.s
				BYPRODUCTS jmp0.bin
			)

add_custom_target(bios DEPENDS bios.bin)
add_custom_target(interrupts DEPENDS interrupts.bin)
add_custom_target(jmp DEPENDS jmp.bin)
add_custom_target(jmp0 DEPENDS jmp0.bin)

add_custom_target(asm-test)
add_dependencies(asm-test bios interrupts jmp jmp0)