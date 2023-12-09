#include <cstdlib>//EXIT_SUCCESS

#include "gui/gui.hpp"

int main (void)
{
	E5150::GUI::init();
	E5150::GUI::UILoop();
	E5150::GUI::clean();
	return EXIT_SUCCESS;
}
