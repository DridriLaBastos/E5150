#ifndef __PORTS_HPP__
#define __PORTS_HPP__

#include "util.hpp"

namespace E5150 { class Component; }

struct PortInfos
{
	const unsigned int addressMask;
	const uint16_t startAddress;
	const uint16_t endAddress;
	E5150::Component* const component;
};

class PORTS
{
	public:
		uint8_t read (const uint16_t port_number) const;
		void write(const uint16_t port_number, const uint8_t data);
		void connect (const PortInfos& portInfos);

	private:
		std::vector<PortInfos> m_portDevices;
};

#endif