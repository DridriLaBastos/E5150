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
			Component (const std::string& name, const unsigned int addressMask):
				m_addressMask(addressMask), m_name(name) {}

			void writeToComponent (const unsigned int address, const uint8_t data)
			{
				const unsigned int localAddress = address & m_addressMask;
				DEBUG("W ({}): {:#x} --> {:#x} (local: {:#b})",m_name,(unsigned)data,address,localAddress);
				write(localAddress, data);
			}

			uint8_t readFromComponent (const unsigned int address)
			{
				const unsigned int localAddress = address & m_addressMask;
				const uint8_t readData = read(localAddress);
				DEBUG("R ({}): {:#x} <-- {:#x}(local: {:#b})",m_name,(unsigned)readData,address,localAddress);
				return readData;
			}

			const unsigned int m_addressMask;
		
			const std::string m_name;
			
		protected:
			virtual void write		(const unsigned int localAddress, const uint8_t data) = 0;
			virtual uint8_t read	(const unsigned int localAddress) = 0;

	};
}

#endif