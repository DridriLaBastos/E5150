#ifndef CONFIG_HPP
#define CONFIG_HPP

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#ifdef DEBUG_BUILD
	//#define STOP_AT_CLOCK
	#define STOP_AT_INSTRUCTION
	#define SEE_CURRENT_INST
	#define SEE_REGS
	#define SEE_FLAGS
	#define SEE_RAM_RW

#if defined(STOP_AT_CLOCK) && defined(STOP_AT_INSTRUCTION)
#error "STOP_AT_CLOCK and STOP_AT_AND cannot be defined together"
#endif
#else
//#define CLOCK_DEBUG
//#define STOP_AT_END
//#define SEE_CURRENT_INST
//#define SEE_REGS
//#define SEE_FLAGS

#endif//CONFIG_HPP

#endif//CONFIG_HPP
