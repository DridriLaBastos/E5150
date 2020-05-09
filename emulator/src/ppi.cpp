#include "ppi.hpp"

namespace E5150
{
	//TODO: change those values
	PPI::PPI(PORTS& ports): Component("PPI", 0b11)
	{
		PortInfos info0x60;
		info0x60.component	= this;
		info0x60.portNum	= 0x60;

		PortInfos info0x61;
		info0x61.component	= this;
		info0x61.portNum	= 0x62;

		PortInfos info0x62;
		info0x62.component	= this;
		info0x62.portNum	= 0x62;

		PortInfos info0x63;
		info0x63.component	= this;
		info0x63.portNum	= 0x63;

		ports.connect(info0x60);
		ports.connect(info0x61);
		ports.connect(info0x62);
		ports.connect(info0x63);
	}

	void PPI::write(const unsigned int localAddress, const uint8_t data)
	{
	}

	uint8_t PPI::read (const unsigned int addr)
	{
	}
}