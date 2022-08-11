#include "core/ppi.hpp"

namespace E5150
{
	//TODO: change those values
	PPI::PPI(PORTS& ports): Component("PPI",ports,0x60,0b11)
	{
	}

	void PPI::write(const unsigned int localAddress, const uint8_t data)
	{
	}

	uint8_t PPI::read (const unsigned int addr)
{ return 0;
	}
}
