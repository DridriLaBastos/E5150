#ifndef COMMON_PCH
#define COMMON_PCH

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
#include <cassert>
#include <iostream>
#include <algorithm>
#include <cinttypes>
#include <exception>
#include <functional>
#include <filesystem>
#include <string_view>

extern "C"
{
	#include "xed/xed-interface.h"
	#include "xed/xed-operand-visibility-enum.h"
}

#include "spdlog/spdlog.h"

#endif
