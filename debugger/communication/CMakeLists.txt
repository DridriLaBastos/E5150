project(emulator_python_communication
        LANGUAGES C)

file(GLOB COMMUNICATION_DYLIB_SRC CONFIGURATION_DEPENDS *.c)

add_library(decom SHARED ${COMMUNICATION_DYLIB_SRC})
