#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include "util.hpp"
#include "ports.hpp"

namespace E5150
{
	class Component
	{
		public:
			Component (const std::string& name, PORTS& ports, const uint16_t portStartAddress, const unsigned int portAddressMask):
				m_addressMask(portAddressMask), m_name(name)
				{
					//ports.connect({.addressMask=portAddressMask, .component=this, .endAddress=(uint16_t)(portStartAddress + portAddressMask), .startAddress=portStartAddress});
					ports.connect(PortInfos{portAddressMask,portStartAddress, (uint16_t)(portAddressMask + portAddressMask), this});
				}
			
			virtual void write		(const unsigned int localAddress, const uint8_t data) = 0;
			virtual uint8_t read	(const unsigned int localAddress) = 0;

			const unsigned int m_addressMask;
		
			const std::string m_name;
	};
}

#endif