#include <thread>
#include <signal.h>
#include <SFML/System/Clock.hpp>

#include "arch.hpp"
#include "util.hpp"

using FloatMicroDuration = std::chrono::duration<float, std::micro>;

static constexpr unsigned int CLOCK_PER_BLOCKS = 1500000;
static constexpr unsigned int BASE_CLOCK = 14318181;
static constexpr unsigned int FDC_CLOCK_MUL = 839;

static const sf::Time TIME_PER_BLOCK = sf::seconds((float)CLOCK_PER_BLOCKS * 1.f/BASE_CLOCK);

static void stop(const int signum)
{
	E5150::Util::_continue = false;
	INFO("Simulation stopped by 'signal {}'", signum);
}

E5150::Arch::Arch(): m_ram(), m_cpu(m_ram, m_ports), m_pic(m_ports, m_cpu), m_pit(m_ports, m_pic), m_ppi(m_ports),m_fdc(m_pic,m_ports)
{
	INFO("Welcome to E5150, the emulator of an IBM PC 5150");
	#ifndef STOP_AT_END
		INFO("Configured : {} clk per block - {} ms per block", CLOCK_PER_BLOCKS,TIME_PER_BLOCK.asMilliseconds());
	#endif
	INFO("This program use the library Intel XED to decode the instructions");
	INFO("This library is accessible at : https://intelxed.github.io");
	INFO("xed version : {}\n",xed_get_version());

	E5150::Util::_continue = true;
	E5150::Util::_stop = true;
	E5150::Util::CURRENT_DEBUG_LEVEL = DEBUG_LEVEL_MAX;
	E5150::Util::undef = (unsigned int)(unsigned long)(&m_ram);

	signal(SIGKILL, stop);
	signal(SIGSTOP, stop);
	signal(SIGQUIT, stop);
	signal(SIGABRT, stop);
	signal(SIGINT, stop);
}

RAM& E5150::Arch::getRam() { return m_ram; }

static void clockWait()
{
	if (E5150::Util::_stop)
	{
		if (E5150::Util::CURRENT_DEBUG_LEVEL == 0)
			E5150::Util::CURRENT_DEBUG_LEVEL = DEBUG_LEVEL_MAX;

		std::string tmp;
		std::getline(std::cin, tmp);
		if (tmp == "q")
			E5150::Util::_continue = false;
		
		if (tmp == "c")
		{
			E5150::Util::CURRENT_DEBUG_LEVEL = 0;
			E5150::Util::_stop=false;
		}
	}
}

void E5150::Arch::startSimulation()
{
	sf::Clock clock;
	sf::Time timeForAllBlocks = sf::Time::Zero;
	unsigned int blockCount = 0;
	unsigned int currentClock = 0;
	unsigned int masterClock = 0;
	unsigned int fdcClock = 0;

	try
	{
		clock.restart();
		while (Util::_continue)
		{
			//The simulation simulates blocks of clock instead of raw clock ticks, otherwise the times are too small to be accurately measured.
			//The next block is launch if we have enougth time (we can run at less clock than specified but not more)
			unsigned int clockToExecute = CLOCK_PER_BLOCKS;
			const unsigned int clocksLeftAfterThisBlock = BASE_CLOCK - (currentClock + CLOCK_PER_BLOCKS);
			
			if (clocksLeftAfterThisBlock < CLOCK_PER_BLOCKS)
				clockToExecute += clocksLeftAfterThisBlock;
			
			currentClock += clockToExecute;

			const sf::Time blockBegin = clock.getElapsedTime();
			for (size_t clock = 0; (clock < clockToExecute) && Util::_continue; ++clock)
			{
				++masterClock;

				#if defined(STOP_AT_END) || defined(CLOCK_DEBUG)
				const bool instructionExecuted = m_cpu.decode();

				if (instructionExecuted)
				{
					if (E5150::Util::_stop)
					{
						m_cpu.printRegisters();
						m_cpu.printFlags();
						m_cpu.printCurrentInstruction();
						clockWait();
					}
				}
				m_cpu.exec();
				#else
					m_cpu.clock();
				#endif

				m_pit.clock();

				while (((fdcClock+1)*1000 <= masterClock*FDC_CLOCK_MUL) && ((fdcClock+1) <= 4000000))
				{
					++fdcClock;
					m_fdc.clock();
				}
			}

		#if !defined(STOP_AT_END) && !defined(CLOCK_DEBUG)
			const sf::Time blockEnd = clock.getElapsedTime();
			++blockCount;
			const sf::Time timeForBlock = blockEnd - blockBegin;
			timeForAllBlocks += timeForBlock;
			const sf::Int64 microsecondsToWait = TIME_PER_BLOCK.asMicroseconds() - timeForBlock.asMicroseconds();
			if (microsecondsToWait > 0)
				std::this_thread::sleep_for(std::chrono::microseconds(microsecondsToWait));
		#endif

			if (clock.getElapsedTime() >= sf::seconds(1.f))
			{
			#if not defined(DEBUG_BUILD)
				const float clockAccurency = (float)currentClock/(float)BASE_CLOCK*100.f;
				const float fdcClockAccurency = (float)fdcClock/4e6*100.f;
				std::cout << "clock executed: " << masterClock << " / " << BASE_CLOCK << std::endl;
				std::cout << "\tclock accurency: " << clockAccurency << "%\n";
				std::cout << "\tfdc clock accurency: " << fdcClockAccurency << "%\n";
				std::cout << "blocks: " << blockCount << "/" << BASE_CLOCK/CLOCK_PER_BLOCKS << " "
					<< timeForAllBlocks.asMicroseconds()/blockCount  << "us (" << timeForAllBlocks.asMilliseconds()/blockCount
					<< "ms) / block - real time: " << TIME_PER_BLOCK.asMicroseconds() << "us (" << TIME_PER_BLOCK.asMilliseconds() << "ms)\n";
				std::cout << "instructions executed: " << (float)m_cpu.instructionExecuted/1e6 << "M" << '\n' << std::endl;
				timeForAllBlocks = sf::Time::Zero;
				clock.restart();
			#endif
				blockCount = 0;
				currentClock = 0;
				masterClock = 0;
				fdcClock = 0;
				m_cpu.instructionExecuted = 0;
			}
		}
	}
	catch (const std::exception& e)
	{ ERROR(e.what()); }
	INFO("Simulation quit !");
}