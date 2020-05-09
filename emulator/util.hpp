#ifndef UTIL_HPP
#define UTIL_HPP

#include "pch.hpp"
#include "config.hpp"

namespace E5150
{
	struct  Util
	{ static bool _continue; };
	
}

#if defined(DEBUG) || defined(STOP_AT_END) || defined(CLOCK_DEBUG)
	#define CLOCK_PAUSE
#endif

#ifdef CLOCK_PAUSE
	#define PAUSE { std::string tmp; std::getline(std::cin, tmp); if (tmp == "q") E5150::Util::_continue = false; }
#else
	#define PAUSE
#endif

#ifdef DEBUG
	#define ASSERT(x) assert(x)
#else
	#define ASSERT(x)
#endif

#endif