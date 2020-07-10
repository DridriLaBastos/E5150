#ifndef CONFIG_HPP
#define CONFIG_HPP

#define DEBUG_BUILD

#ifdef DEBUG_BUILD
	#define CLOCK_DEBUG
	#define STOP_AT_END
	#define SEE_CURRENT_INST
	#define SEE_REGS
	#define SEE_FLAGS
#else
//#define CLOCK_DEBUG
//#define STOP_AT_END
//#define SEE_CURRENT_INST
//#define SEE_REGS
//#define SEE_FLAGS

#endif

#endif