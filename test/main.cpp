#include "arch.hpp"

/* Exemple de fonctionnement */
int main (void)
{
	E5150::Arch arch;

	arch.getRam().load("jmp.bin", 0xFFFF0);
	arch.getRam().load("bios.bin", 0xE0000);
	arch.getRam().load("test.bin", 0x100);

	arch.startSimulation();

	return EXIT_SUCCESS;
}
