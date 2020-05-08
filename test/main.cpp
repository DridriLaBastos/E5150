#include "log.hpp"
#include "arch.hpp"

/* Exemple de fonctionnement */
int main (void)
{
	E5150::Arch arch;

	//Loading bochs BIOS
	//arch.getRam().load("/Users/adrien/dev/share/bochs/BIOS-bochs-legacy", 0xF0000);
	arch.getRam().load("jmp.bin", 0xFFFF0);
	arch.getRam().load("bios.bin", 0xE0000);
	arch.getRam().load("test.bin", 0x1000);

	arch.startSimulation();

	return EXIT_SUCCESS;
}
