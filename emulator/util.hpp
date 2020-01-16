#ifndef UTIL_HPP
#define UTIL_HPP

/**
 * BUILD is defined at compile time.
 * Because clang doesn't search PCH files included with include we have to use -include in command line.
 * But if we remove the #include line, vscode intellisense doesn't work, so we removed the line at compile time to use PCH
 */
#ifndef BUILD
	#include "pch.hpp"
#endif

#include "config.hpp"

#if defined(DEBUG) || defined(STOP_AT_END) || defined(CLOCK_DEBUG)
	#define CLOCK_PAUSE
#endif

//I first implement this with a call to exit if q was enterred. But a call to exit
//doesn't imply a call to the destructors and thus any heap allocated ressources isn't garrantied to be freed, so 
//I rewrite this using exception even if enterring q is not really an exception. 
#ifdef CLOCK_PAUSE
	struct Exit: public std::exception
	{ const char* what (void) const noexcept { return "Exit"; }	};

	#define PAUSE { std::string tmp; std::getline(std::cin, tmp); if (tmp == "q") throw Exit(); }
#else
	#define PAUSE
#endif

#ifdef DEBUG
	#define ASSERT(x) assert(x)
#else
	#define ASSERT(x)
#endif

#endif