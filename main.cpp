#include "gui/gui.hpp"

/* Exemple de fonctionnement */
int main (const int argc, const char* argv [])
{
	E5150::GUI::PLATFORM::init(argc,argv);
	E5150::GUI::init();

	E5150::GUI::PLATFORM::UILoop();

	E5150::GUI::clean();
	E5150::GUI::PLATFORM::clean();
	return EXIT_SUCCESS;
}
