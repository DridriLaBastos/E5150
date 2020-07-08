#ifndef PCH_HPP
#define PCH_HPP

#include <set>
#include <array>
#include <string>
#include <memory>
#include <vector>
#include <utility>
#include <fstream>
#include <cstdint>
#include <stdint.h>
#include <iostream>
#include <algorithm>
#include <exception>
#include <functional>
#include <SFML/System.hpp>

extern "C"
{
	#include "xed/xed-interface.h"
	#include "xed/xed-operand-visibility-enum.h"
}

#include "spdlog/spdlog.h"

#endif