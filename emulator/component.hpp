#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include "stdint.h"
#include "ports.hpp"

namespace E5150
{
	class Component
	{
		public:
			virtual void write		(const unsigned int address, const uint8_t data) = 0;
			virtual uint8_t read	(const unsigned int address) = 0;
	};
}

#endif