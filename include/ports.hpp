#ifndef __PORTS_HPP__
#define __PORTS_HPP__

#include "util.hpp"

namespace E5150 { class Component; }

struct PortInfos
{
	uint16_t portNum;
	E5150::Component* component;
};

class PORTS
{
	public:
		uint8_t read (const uint16_t port_number) const;
		void write(const uint16_t port_number, const uint8_t data);
		void connect (const PortInfos& portInfos);

	private:
		std::set<PortInfos> m_portDevices;
};

#endif