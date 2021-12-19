#include "arch.hpp"
#include "ports.hpp"
#include "component.hpp"

#define PORTSDebug(REQUIRED_DEBUG_LEVEL,...) debug<REQUIRED_DEBUG_LEVEL>("PORTS: " __VA_ARGS__)

uint8_t PORTS::read (const uint16_t portAddress) const 
{
	uint8_t data = dataBus;
	const auto& found = std::find_if(m_portDevices.begin(), m_portDevices.end(),
	[portAddress](const PortInfos& portInfo) { return (portAddress >=  portInfo.startAddress) && (portAddress <= portInfo.endAddress); });

	if (found != m_portDevices.end())
	{
		const PortInfos& portInfo = *found;
		const unsigned int componentLocalAddress = portAddress & portInfo.addressMask;
		portInfo.component->read(portAddress & found->addressMask);
		#ifdef SEE_PORT_READ
			PORTSDebug(DEBUG_LEVEL_MAX,"R ({}): {:#x} <-- {:#x} (local: {:#b})",portInfo.component->m_name,(unsigned)data,portAddress,componentLocalAddress);
		#endif
	}
	else
		PORTSDebug(8,"No device found at port address {:#x}. Result is watever is on the data bus ({:#x})",portAddress,data);

	return data;
}

void PORTS::write(const uint16_t portAddress, const uint8_t data) 
{
	dataBus = data;
	const auto& found = std::find_if(m_portDevices.begin(), m_portDevices.end(),
	[portAddress](const PortInfos& portInfo) { return (portAddress >=  portInfo.startAddress) && (portAddress <= portInfo.endAddress); });

	if (found != m_portDevices.end())
	{
		const PortInfos& portInfo = *found;
		const unsigned int componentLocalAddress = portAddress & portInfo.addressMask;
		#ifdef SEE_PORT_WRITE
			PORTSDebug(DEBUG_LEVEL_MAX,"W ({}): {:#x} --> {:#x} (local: {:#b})",portInfo.component->m_name,(unsigned)data,portAddress,componentLocalAddress);
		#endif
	}
	else
		PORTSDebug(8,"No device found at port address {:#x}",portAddress);
}

static bool inBox (const unsigned int begin1, const unsigned int end1, const unsigned int begin2, const unsigned int end2)
{
	const unsigned int firstIntervalSize = end1 - begin1;

	return (begin2 - begin1 > firstIntervalSize) || begin1 > end2;
}

void PORTS::connect (const PortInfos& portInfos)
{
	for (const auto& portDevice: m_portDevices)
		assert(inBox(portInfos.startAddress,portInfos.endAddress,portDevice.startAddress,portDevice.endAddress));

	m_portDevices.emplace_back(portInfos);
}
