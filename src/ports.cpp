#include "ports.hpp"
#include "component.hpp"

uint8_t PORTS::read (const uint16_t portAddress) const 
{
	uint8_t data;
	bool readDone = false;
	for (const auto& portInfo: m_portDevices)
	{
		if ((portAddress >= portInfo.startAddress) && (portAddress <= portInfo.endAddress))
		{
			const unsigned int componentLocalAddress = portAddress & portInfo.addressMask;
			data = portInfo.component->read(componentLocalAddress);
			DEBUG("R ({}): {:#x} <-- {:#x} (local: {:#b})",portInfo.component->m_name,(unsigned)data,portAddress,componentLocalAddress);
			readDone = true;
			break;
		}
	}
	
	if (!readDone)
		debug<4>("No device found at port address {:#x}. Result undetermined",portAddress);

	return data;
}

void PORTS::write(const uint16_t portAddress, const uint8_t data) 
{
	for (const auto& portInfo: m_portDevices)
	{
		if ((portAddress >= portInfo.startAddress) && (portAddress <= portInfo.endAddress))
		{
			const unsigned int componentLocalAddress = portAddress & portInfo.addressMask;
			DEBUG("W ({}): {:#x} --> {:#x} (local: {:#b})",portInfo.component->m_name,(unsigned)data,portAddress,componentLocalAddress);
			portInfo.component->write(componentLocalAddress,data);
			return;
		}
	}

	debug<4>("No device found at port address {:#x}",portAddress);
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
