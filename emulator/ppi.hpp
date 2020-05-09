#ifndef PPI_HPP
#define PPI_HPP

#include "util.hpp"
#include "component.hpp"

namespace E5150
{
	class PPI: public E5150::Component
	{
		public:
			PPI(PORTS& ports);
			virtual uint8_t read (const unsigned int localAddress) final;
			virtual void write (const unsigned int localAddress, const uint8_t data) final;
	};
}

#endif