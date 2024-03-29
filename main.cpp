#include "arch.hpp"

/* Exemple de fonctionnement */
int main (void)
{
#ifdef DEBUG_BUILD
	spdlog::set_level(spdlog::level::trace);
#endif
	//Well... ty for that C++ -.-
	E5150::Util::undef = (unsigned long)main & (unsigned int)-1;
	E5150::Arch arch;

	//Loading bochs BIOS
	//arch.getRam().load("/Users/adrien/dev/share/bochs/BIOS-bochs-legacy", 0xF0000);
	RAM& ram = arch.getRam();
	ram.load("test/interrupts.bin",0);
	ram.load("test/jmp.bin", 0xFFFF0);
	ram.load("test/bios.bin",0xE0000);
	std::cout << std::showbase;
	arch.startSimulation();
	std::cout << std::noshowbase;
	return EXIT_SUCCESS;
}

