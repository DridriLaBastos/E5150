#ifndef ARCH_HPP
#define ARCH_HPP

#if 0
#include "pic.hpp"
#include "pit.hpp"
#include "ram.hpp"
#include "ppi.hpp"
#include "8086.hpp"
#include "ports.hpp"
#include "fdc.hpp"

#define addressBus E5150::Arch::_addressBus
#define dataBus E5150::Arch::_dataBus
#endif

#include "core/bus.hpp"
#include "core/ram.hpp"
#include "core/8086.hpp"

namespace E5150
{
	class Arch
	{
	public:
		//TODO: Need thread safety ?
		struct EmulationStat {
			uint64_t cpuClock, fdcClock, instructionExecutedCount;
		};

		public:
			Arch(void);

			void SimulationLoop(void);
			void StopSimulation(void);

		public:
		#if 0
			static RAM ram;
			static PORTS ports;
			static CPU cpu;
			static PIC pic;
			static PIT pit;
			static PPI ppi;
			static FDC fdc;
		#endif
#if 0
			static BUS<20> _addressBus;
			static BUS<8> _dataBus;
#endif
	public:
		static Intel8088 cpu;
		static RAM ram;

		static EmulationStat emulationStat;
		static E5150::BUS<20> addressBus;
		static E5150::BUS<8> dataBus;
	};
}

#endif