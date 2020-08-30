#ifndef CONFIG_HPP
#define CONFIG_HPP

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#ifdef DEBUG_BUILD
	#define CLOCK_DEBUG
	#define STOP_AT_END
	#define SEE_CURRENT_INST
	#define SEE_REGS
	#define SEE_FLAGS
	#define SEE_RAM_RW
#else
//#define CLOCK_DEBUG
//#define STOP_AT_END
//#define SEE_CURRENT_INST
//#define SEE_REGS
//#define SEE_FLAGS

#endif//DEBUG_BUILD

#endif//CONFIG_HPP
