#ifndef GUI_HPP
#define GUI_HPP

namespace E5150::GUI {
	int platformInit (int argc, const char* argv[]);
	void platformUILoop(void);
	void guiInit(void);
	void drawGui(void);
}

#endif