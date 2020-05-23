#ifndef PCH_HPP
#define PCH_HPP

#include <set>
#include <array>
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <cstdint>
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

#include "third-party/spdlog/include/spdlog/spdlog.h"

#endif