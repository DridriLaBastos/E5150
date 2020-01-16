#ifndef ARCH_HPP
#define ARCH_HPP

#include "util.hpp"

#include "pic.hpp"
#include "pit.hpp"
#include "ram.hpp"
#include "dma.hpp"
#include "8086.hpp"
#include "ports.hpp"

namespace E5150
{
	class Arch
	{
		public:
			Arch(void);

			void startSimulation(void);
			RAM& getRam(void);

		private:
			RAM m_ram;
			PORTS m_ports;
			CPU m_cpu;
			PIC m_pic;
			PIT m_pit;
			DMA m_dma;
	};
}

#endif