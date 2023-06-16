#ifndef PCH_HPP
#define PCH_HPP

#include <set>
#include <array>
#include <atomic>
#include <thread>
#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <future>
#include <utility>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <exception>
#include <functional>
#include <string_view>

extern "C"
{
	#include "xed/xed-interface.h"
	#include "xed/xed-operand-visibility-enum.h"
}

#include "spdlog/spdlog.h"

#endif
