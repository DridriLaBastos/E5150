#ifndef UTIL_HPP
#define UTIL_HPP

constexpr unsigned int EMULATION_MAX_LOG_LEVEL = 10;

namespace E5150
{
	namespace Util
	{
		extern unsigned int CURRENT_EMULATION_LOG_LEVEL;
		extern bool _continue;
		extern bool _stop;
		extern unsigned int undef;//For all undefined value, it is set to a random value at runtime
	};
}

using Milliseconds = std::chrono::milliseconds;
using Clock = std::chrono::high_resolution_clock;

#define E5150_INFO(...) spdlog::info(__VA_ARGS__)
#define E5150_WARNING(...) spdlog::warn(__VA_ARGS__)
#define E5150_ERROR(...) spdlog::error(__VA_ARGS__)
#define E5150_DEBUG(...) spdlog::debug(__VA_ARGS__)

template <unsigned int REQUIRED_LOG_LEVEL, class... Args>
void EMULATION_INFO_LOG(Args&&... args)
{
#ifdef DEBUGGER
	static_assert(REQUIRED_LOG_LEVEL > 0, "Log level of 0 is reserved for no log at all");
	if (REQUIRED_LOG_LEVEL <= E5150::Util::CURRENT_EMULATION_LOG_LEVEL)
		E5150_DEBUG(args...);
#endif
}

#ifndef _WIN32
	#define FORCE_INLINE inline __attribute__((always_inline))
#else
	#define FORCE_INLINE
#endif

#endif