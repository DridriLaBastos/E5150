project(emulator_python_communication
        LANGUAGES C)

if(APPLE OR UNIX)
	set(PLATFORM_DEPENDANT communication_unix.c)
endif(APPLE OR UNIX)


add_library(decom SHARED command.c ${PLATFORM_DEPENDANT})
