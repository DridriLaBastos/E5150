#include "core/include/arch.hpp"

/* Exemple de fonctionnement */
int main (void)
{
#ifdef DEBUG_BUILD
	spdlog::set_level(spdlog::level::trace);
#endif
	E5150::Arch arch;

	#if 1
		//Loadin IBM BIOS
		ram.load("test/ibm_bios.bin", 0xFE000);
	#else
		//Loading custom test code
		ram.load("test/interrupts.bin",0);
		ram.load("test/jmp.bin", 0xFFFF0);
		//ram.load("/Users/adrien/Documents/Informatique/OS/Beetle16/init/init.bin",0x500);
		ram.load("test/bios.bin",0x500);
	#endif

	arch.startSimulation();
	return EXIT_SUCCESS;
}
