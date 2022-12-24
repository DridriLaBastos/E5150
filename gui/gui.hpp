#ifndef GUI_HPP
#define GUI_HPP

namespace E5150::GUI {
	namespace PLATFORM {
		int init (const int argc, const char** argv);
		void UILoop(void);
		void clean(void);
	}
	void init(void);
	void draw(void);
	void clean(void);
}

#endif