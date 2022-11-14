#include <cstddef>
#include <cstdint>

#include "core/arch.hpp"
#include "gui/gui.hpp"

/* Exemple de fonctionnement */
int main (const int argc, const char* argv [])
{
#ifdef DEBUGGER
	E5150_INFO("Emulation with debugger (loglevel: {})",E5150::Util::CURRENT_EMULATION_LOG_LEVEL);
	spdlog::set_level(spdlog::level::debug);
#endif
	//E5150::Arch arch;

	#if 1
		//Loading IBM BIOS
		ram.load("test/ibm_bios.bin", 0xFE000);
	#else
		//Loading custom test code
		ram.load("test/interrupts.bin",0);
		ram.load("test/jmp.bin", 0xFFFF0);
		//ram.load("/Users/adrien/Documents/Informatique/OS/Beetle16/init/init.bin",0x500);
		ram.load("test/bios.bin",0x500);
	#endif

	//arch.startSimulation();
	E5150::GUI::platformInit(argc,argv);
	E5150::GUI::guiInit();
	E5150::GUI::platformUILoop();
	return EXIT_SUCCESS;
}
