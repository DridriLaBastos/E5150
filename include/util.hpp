#ifndef UTIL_HPP
#define UTIL_HPP

#include "pch.hpp"
#include "config.hpp"

constexpr unsigned int DEBUG_LEVEL_MAX = 10;
constexpr unsigned int CURRENT_DEBUG_LEVEL = DEBUG_LEVEL_MAX;

namespace E5150
{
	struct  Util
	{
		static bool _continue;
		static bool _stop;
	};
}

using Milliseconds = std::chrono::milliseconds;
using Clock = std::chrono::high_resolution_clock;

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

#ifdef DEBUG_BUILD
	#define ASSERT(x) assert(x)
#else
	#define ASSERT(x)
#endif

template <unsigned int DEBUG_LEVEL_REQUIRED,class... Args>
void debug(Args&&... args)
{
	static_assert(CURRENT_DEBUG_LEVEL <= DEBUG_LEVEL_MAX);
	if (CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_REQUIRED) DEBUG(args...);
}
#endif