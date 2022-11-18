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
		struct EmulationStat {
			std::atomic<uint64_t> cpuClock, fdcClock, instructionExecutedCount;
		};

		public:
			Arch(void);

			void startSimulation(void);

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

			static EmulationStat emulationStat;

			static constexpr unsigned int CPU_BASE_CLOCK = 14318181;
			static constexpr unsigned int FDC_BASE_CLOCK =  4000000;
			static constexpr unsigned int CLOCK_PER_BLOCK = 1500000;
			static constexpr unsigned int NS_PER_CLOCK = 1.f/CPU_BASE_CLOCK*1e9+.5f;
			static constexpr unsigned int BLOCK_PER_SECOND = CPU_BASE_CLOCK / CLOCK_PER_BLOCK;
	};
}

#endif