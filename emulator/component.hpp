#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include "util.hpp"
#include "ports.hpp"

namespace E5150
{
	class Component
	{
		public:
			Component (const std::string& name, const unsigned int writeBusSize, const unsigned int readBusSize):
				m_writeBusSize(writeBusSize), m_readBusSize(readBusSize), m_name(name) {}

			void writeToComponent (const unsigned int address, const uint8_t data)
			{
				const unsigned int localAddress = address & m_writeBusSize;
				#ifdef DEBUG
					std::cout << "W (" << m_name << "): " << std::showbase << std::hex << (unsigned)data << " --> " << localAddress << "(" << address << ")" << std::noshowbase << std::dec << std::endl;
				#endif
				write(localAddress, data);
			}

			uint8_t readFromComponent (const unsigned int address)
			{
				const unsigned int localAddress = address & m_readBusSize;
				const uint8_t readData = read(localAddress);
				#ifdef DEBUG
					std::cout << "R (" << m_name << "): " << std::showbase << std::hex << (unsigned)readData << " <--  " << localAddress << "(" << address << ")" << std::noshowbase << std::dec << std::endl;
				#endif
				return readData;
			}

			const unsigned int m_writeBusSize;
			const unsigned int m_readBusSize;
		
		protected:
			virtual void write		(const unsigned int localAddress, const uint8_t data) = 0;
			virtual uint8_t read	(const unsigned int localAddress) = 0;
		
		private:
			std::string m_name;
	};
}

#endif