#ifndef UTIL_HPP
#define UTIL_HPP

constexpr unsigned int DEBUG_LEVEL_MAX = 10;

namespace E5150
{
	namespace Util
	{
		extern unsigned int CURRENT_DEBUG_LEVEL;
		extern bool _continue;
		extern bool _stop;
		extern unsigned int undef;//For all undefined value, it is set to a random value at runtime
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
#define WARNING(...)	spdlog::warn(__VA_ARGS__)
#define ERROR(...)		spdlog::error(__VA_ARGS__)

#ifdef DEBUG_BUILD
	#define ASSERT(x) assert(x)
#else
	#define ASSERT(x)
#endif

#define FORCE_INLINE inline __attribute__((always_inline))

//Print nothing if not in debug build
template <unsigned int DEBUG_LEVEL_REQUIRED,class... Args>
void debug(Args&&... args)
{
	ASSERT(E5150::Util::CURRENT_DEBUG_LEVEL <= DEBUG_LEVEL_MAX);
	if (E5150::Util::CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_REQUIRED) DEBUG(args...);
}
#endif
