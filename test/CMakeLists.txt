project(test)

enable_language(ASM_NASM)

set(CMAKE_ASM_NASM_OBJECT_FORMAT bin)
set(CMAKE_ASM_NASM_FLAGS "-I ${CMAKE_SOURCE_DIR}")

if (NOT EXISTS ${PROJECT_BINARY_DIR}/ibm_bios.bin)
	file(DOWNLOAD http://www.minuszerodegrees.net/bios/BIOS_IBM5150_24APR81_5700051_U33.BIN ${PROJECT_BINARY_DIR}/ibm_bios.bin SHOW_PROGRESS)
	file(SHA256 ${PROJECT_BINARY_DIR}/ibm_bios.bin IBM_BIOS_CHECKSUM)
	set(IBM_BIOS_EXPECTED_CHECKSUM "a259306c47b43e14ef6378d7bdeff67331a2519d363bc36b3f354bc91a6874c8")

	message(CHECK_START "Downloaded BIOS file integrity")
	if (${IBM_BIOS_CHECKSUM} STREQUAL ${IBM_BIOS_EXPECTED_CHECKSUM})
		message(CHECK_PASS "OK")
	else()
		message(CHECK_FAIL "Downloaded BIOS file contains an error")
		message(AUTHOR_WARNING "Excpected checksum: ${IBM_BIOS_EXPECTED_CHECKSUM} got ${IBM_BIOS_CHECKSUM}")
	endif()
else()
	message(STATUS "BIOS file already downloaded")
endif()

add_custom_command(OUTPUT bios.bin
 					COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/bios.s -o bios.bin
                    MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/bios.s
 				)

 add_custom_command(OUTPUT interrupts.bin
 					COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/interrupts.s -o interrupts.bin
		 			MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/interrupts.s
 				)

 add_custom_command(OUTPUT jmp.bin
 					COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/jmp.s -o jmp.bin
		 			MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/jmp.s
         )

 add_custom_command(OUTPUT jmp0.bin
 				COMMAND ${CMAKE_ASM_NASM_COMPILER} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} ${CMAKE_ASM_NASM_FLAGS} ${PROJECT_SOURCE_DIR}/jmp0.s -o jmp0.bin
		 		MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/jmp0.s
		 )

 add_custom_target(asm-test SOURCES bios.bin interrupts.bin jmp.bin jmp0.bin)
