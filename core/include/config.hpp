#ifndef CONFIG_HPP
#define CONFIG_HPP

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
