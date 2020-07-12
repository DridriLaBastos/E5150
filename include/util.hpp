#ifndef UTIL_HPP
#define UTIL_HPP

#include "pch.hpp"
#include "config.hpp"

namespace E5150
{
	struct  Util
	{
		static bool _continue;
	};
}

#ifdef DEBUG_BUILD
	#define DEBUG(...)		spdlog::debug(__VA_ARGS__)
#else
	#define DEBUG(...)
#endif
#define INFO(...)		spdlog::info(__VA_ARGS__)
#define WARNING(...)	spdlog::warning(__VA_ARGS__)
#define ERROR(...)		spdlog::error(__VA_ARGS__)

#if defined(DEBUG_BUILD) || defined(STOP_AT_END) || defined(CLOCK_DEBUG)
	#define CLOCK_PAUSE
#endif

#ifdef CLOCK_PAUSE
	#define PAUSE { std::string tmp; std::getline(std::cin, tmp); if (tmp == "q") E5150::Util::_continue = false; }
#else
	#define PAUSE
#endif

#ifdef DEBUG_BUILD
	#define ASSERT(x) assert(x)
#else
	#define ASSERT(x)
#endif

#endif