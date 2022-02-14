#ifndef ARCH_HPP
#define ARCH_HPP

#include "pic.hpp"
#include "pit.hpp"
#include "ram.hpp"
#include "ppi.hpp"
#include "8086.hpp"
#include "ports.hpp"
#include "fdc.hpp"

#define cpu E5150::Arch::_cpu
#define ram E5150::Arch::_ram
#define pic E5150::Arch::_pic
#define fdc E5150::Arch::_fdc
#define ppi E5150::Arch::_ppi
#define pit E5150::Arch::_pit
#define pit E5150::Arch::_pit
#define ports E5150::Arch::_ports
#define addressBus E5150::Arch::_addressBus
#define dataBus E5150::Arch::_dataBus

namespace E5150
{
	class Arch
	{
		public:
			Arch(void);

			void startSimulation(void);
		
		private:
			void displayCPUStatusAndWait (void) const;
			void wait (void) const;

		public:
			static RAM _ram;
			static PORTS _ports;
			static CPU _cpu;
			static PIC _pic;
			static PIT _pit;
			static PPI _ppi;
			static FDC _fdc;
			static BUS<20> _addressBus;
			static BUS<8> _dataBus;
	};
}

#endif