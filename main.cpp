#include "gui/gui.hpp"

/* Exemple de fonctionnement */
int main (const int argc, const char* argv [])
{
	E5150::GUI::platformInit(argc,argv);
	E5150::GUI::guiInit();
	E5150::GUI::platformUILoop();
	E5150::GUI::guiDeinit();
	return EXIT_SUCCESS;
}
