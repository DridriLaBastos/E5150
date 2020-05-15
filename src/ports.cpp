#include "ports.hpp"
#include "component.hpp"

static bool operator< (const PortInfos& left, const PortInfos& right) { return left.portNum < right.portNum; }

uint8_t PORTS::read (const uint16_t port_number) const 
{
	PortInfos toFind; toFind.portNum = port_number;
	auto found = m_portDevices.find(toFind);

	if (found != m_portDevices.end())
		return found->component->readFromComponent(port_number);
	
	return 0;
}

void PORTS::write(const uint16_t port_number, const uint8_t data) 
{
	PortInfos toFind; toFind.portNum = port_number;
	auto found = m_portDevices.find(toFind);

	if (found != m_portDevices.end())
		found->component->writeToComponent(port_number,data);
}

void PORTS::connect (const PortInfos& portInfos)
{ m_portDevices.emplace(portInfos); }
