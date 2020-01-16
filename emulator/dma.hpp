#ifndef DMA_HPP
#define DMA_HPP

#include "util.hpp"
#include "component.hpp"

namespace E5150
{
	class DMA: public Component
	{
		public:
			virtual void write (const unsigned int address, const uint8_t data) final;
			virtual uint8_t read (const unsigned int address) final;
			void clock (void);
	};
}

#endif