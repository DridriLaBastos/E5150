#include "ports.hpp"
#include "component.hpp"

PORTS::PORTS(void){m_portDevices.reserve(5);}

/* TODO: Algorithme par dichotomie pour la recherche */
uint8_t PORTS::read (const uint16_t port_number) const 
{
	auto pos = std::find_if(m_portDevices.begin(), m_portDevices.end(),
	[port_number](const PortInfos portInfos){return portInfos.portNum == port_number;});

	if (pos != m_portDevices.end())
		return pos->component->read(port_number);
	
	return 0;
}

void PORTS::write(const uint16_t port_number, const uint8_t data) 
{
	auto pos = std::find_if(m_portDevices.begin(), m_portDevices.end(),
	[port_number](const PortInfos portInfos){return portInfos.portNum == port_number;});

	if (pos != m_portDevices.end())
		pos->component->write(port_number,data);
}

void PORTS::connect (const PortInfos& portInfos)
{
	auto pos = std::find_if(m_portDevices.begin(), m_portDevices.end(), 
	[portInfos](const PortInfos portI){return portI.portNum > portInfos.portNum;});

	m_portDevices.emplace(pos, portInfos);
}