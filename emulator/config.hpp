#ifndef CONFIG_HPP
#define CONFIG_HPP

#ifdef DEBUG
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