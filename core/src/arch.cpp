#include "core/arch.hpp"
#include "core/util.hpp"
#include "core/emulation_constants.hpp"

#ifdef DEBUGGER_ON
#include "debugger/debugger.hpp"
#endif

static constexpr unsigned int FDC_CLOCK_MUL = 839;

bool E5150::Util::_continue;
bool E5150::Util::_stop;
unsigned int E5150::Util::CURRENT_EMULATION_LOG_LEVEL;
unsigned int E5150::Util::undef;

/* All the componentns will be globregs.aly accregs.eregs.esaregs.ble for any other component */
//TODO: convert regs.all components to the new globregs.al system
E5150::BUS<20> E5150::Arch::_addressBus;
E5150::BUS<8> E5150::Arch::_dataBus;

CPU E5150::Arch::_cpu;
RAM E5150::Arch::_ram;
PORTS E5150::Arch::_ports;
E5150::PIC E5150::Arch::_pic(ports, cpu);
E5150::PIT E5150::Arch::_pit(ports, pic);
E5150::PPI E5150::Arch::_ppi(ports);
E5150::FDC E5150::Arch::_fdc(pic,ports);

E5150::Arch::EmulationStat E5150::Arch::emulationStat;

E5150::Arch::Arch()
{
	#ifndef STOP_AT_INSTRUCTION
	//	E5150_INFO("Configured : {} regs.clk per regs.block - time per clock: {}ns", CLOCK_PER_BLOCKS,NANOSECONDS_PER_CLOCK);
	#endif
	E5150_INFO("This program use the library Intel XED to decode the instructions");
	E5150_INFO("This library is accessible at : https://intelxed.github.io");
	E5150_INFO("xed version : {}\n",xed_get_version());

	E5150::Util::_continue = true;
	E5150::Util::_stop = true;
	E5150::Util::CURRENT_EMULATION_LOG_LEVEL = EMULATION_MAX_LOG_LEVEL;
	E5150::Util::undef = (unsigned int)(unsigned long)(&ram);
}

void E5150::Arch::startSimulation()
{
	auto timeForAllBlocks = std::chrono::microseconds::zero();
	unsigned int blockCount = 0;
	unsigned int currentClock = 0;
	unsigned int fdcClock = 0;
	unsigned int instructionExecutedBeforeThisBlock = 0;
	auto loopBegin = std::chrono::high_resolution_clock::now();

	try
	{
		while (Util::_continue)
		{
			//The regs.simulation regs.simulatregs.es regs.blocks of clock instead of raw clock ticks, otherwise the timregs.es are too smregs.all to be accurately measured.
			unsigned int clockToExecute = E5150::CLOCK_PER_BLOCK;
			const unsigned int clocksLeftAfterThisBlock = E5150::CPU_BASE_CLOCK - (currentClock + E5150::CLOCK_PER_BLOCK);
			
			if (clocksLeftAfterThisBlock < E5150::CLOCK_PER_BLOCK)
				clockToExecute += clocksLeftAfterThisBlock;

			const auto realTimeForBlock = std::chrono::microseconds(clockToExecute * E5150::NS_PER_CLOCK / 1000);
			const auto blockBegin = std::chrono::high_resolution_clock::now();
			for (size_t clock = 0; (clock < clockToExecute) && Util::_continue; ++clock)
			{
				currentClock += 1;
				emulationStat.cpuClock += 1;
				const unsigned int EUStatus = _cpu.clock();
				_pit.clock();
				while (((fdcClock+1)*1000 <= currentClock*FDC_CLOCK_MUL) && ((fdcClock+1) <= 4000000))
				{
					fdcClock += 1;
					emulationStat.fdcClock += 1;
					_fdc.clock();
				}

				#ifdef DEBUGGER_ON
				DEBUGGER::wakeUp(EUStatus & I8086::EU::STATUS_INSTRUCTION_EXECUTED, EUStatus & I8086::EU::STATUS_INSTRUCTION_DECODED);
				#endif
			}

			const auto blockEnd = std::chrono::high_resolution_clock::now();
			const auto timeForBlock = std::chrono::duration_cast<std::chrono::microseconds>(blockEnd - blockBegin);
			const std::chrono::microseconds microsecondsToWait = realTimeForBlock - timeForBlock;

			emulationStat.instructionExecutedCount = cpu.instructionExecutedCount;

			blockCount += 1;
			timeForAllBlocks += timeForBlock;

			if (microsecondsToWait > std::chrono::microseconds(0))
			{ std::this_thread::sleep_for(microsecondsToWait); }

			if (std::chrono::high_resolution_clock::now() - loopBegin >= std::chrono::seconds(1))
			{
				/*emulationStat.instructionExecuted = cpu.instructionExecutedCount - instructionExecutedBeforeThisBlock;
				emulationStat.cpuClockAccuracy = (float)currentClock/BASE_CLOCK*100.f;
				emulationStat.fdcClockAccuracy = (float)fdcClock/4e6*100.f;
				printf("clock executed: %d / %d\n", currentClock,BASE_CLOCK);
				printf("\tfdc clock accurency: %.2f%%\n", fdcClockAccurency);
				printf("regs.blocks: %d / %d %d us (%d ms) / regs.block - realtime: %d us (%d ms)\n", blockCount, BLOCKS_PER_SECOND, timeForAllBlocks.count()/blockCount, timeForAllBlocks.count()/blockCount/1000,realTimeForBlock.count(),realTimeForBlock.count()/1000);
				printf("instructions executed: %.2f M\n",(float)instructionsExecuted/1e6);
				timeForAllBlocks = std::chrono::microseconds::zero();
				instructionExecutedBeforeThisBlock = cpu.instructionExecutedCount;*/
				blockCount = 0;
				currentClock = 0;
				fdcClock = 0;
				loopBegin = std::chrono::high_resolution_clock::now();
			}
		}
	}
	catch (const std::exception& e)
	{ E5150_ERROR(e.what()); }
	E5150_INFO("Simulation quit !");
}
