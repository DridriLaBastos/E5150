#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include "util.hpp"
#include "ports.hpp"

namespace E5150
{
	//TODO: I am not sure about this public/protected stuff with the read/write functions
	class Component
	{
		public:
			Component (const std::string& name, PORTS& ports, const unsigned int portStartAddress, const unsigned int portAddressMask):
				m_addressMask(portAddressMask), m_name(name)
				{
					PortInfos info;
					info.component = this;
					info.addressMask = portAddressMask;
					info.startAddress = portStartAddress;
					info.endAddress = portStartAddress + portAddressMask;
					ports.connect(info);
				}
			
			virtual void write		(const unsigned int localAddress, const uint8_t data) = 0;
			virtual uint8_t read	(const unsigned int localAddress) = 0;

			const unsigned int m_addressMask;
		
			const std::string m_name;
	};
}

#endif