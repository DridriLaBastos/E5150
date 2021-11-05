#ifndef PCH_HPP
#define PCH_HPP

#include <set>
#include <array>
#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <utility>
#include <fstream>
#include <stdint.h>
#include <iostream>
#include <algorithm>
#include <exception>
#include <functional>

extern "C"
{
	#include "xed/xed-interface.h"
	#include "xed/xed-operand-visibility-enum.h"
}

#include "spdlog/spdlog.h"

#endif
