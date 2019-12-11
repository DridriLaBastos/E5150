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

#endif