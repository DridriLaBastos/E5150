#include <cstddef>//EXIT_SUCCESS

#include "gui/gui.hpp"

int main (const int argc, const char* argv [])
{
	E5150::GUI::init();
	E5150::GUI::UILoop();
	E5150::GUI::clean();
	return EXIT_SUCCESS;
}
