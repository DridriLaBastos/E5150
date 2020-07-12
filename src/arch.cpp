#include <thread>
#include <signal.h>
#include <SFML/System/Clock.hpp>

#include "arch.hpp"
#include "util.hpp"

using Clock = std::chrono::system_clock;
using FloatMicroDuration = std::chrono::duration<float, std::micro>;

static constexpr unsigned int CLOCK_PER_BLOCKS = 10000;
static constexpr unsigned int I8284_CLOCKS_PER_SECOND = 4770000;
static const sf::Time TIME_PER_BLOCK = sf::seconds((float)CLOCK_PER_BLOCKS/I8284_CLOCKS_PER_SECOND);

bool E5150::Util::_continue = true;

static void stop(const int signum)
{
	E5150::Util::_continue = false;
	INFO("Simulation stopped by 'signal {}'", signum);
}

E5150::Arch::Arch(): m_ram(), m_cpu(m_ram, m_ports), m_pic(m_ports, m_cpu), m_pit(m_ports, m_pic), m_ppi(m_ports),m_fdc(m_pic,m_ports)
{
	INFO("Welcome to E5150, the emulator of an IBM PC 5150");
	INFO("This program use the library Intel XED to decode the instructions");
	INFO("This library is accessible at : https://intelxed.github.io");
	INFO("xed version : {} \n",xed_get_version());

	DEBUG("Duration for {} blocks: {}us", CLOCK_PER_BLOCKS,TIME_PER_BLOCK.asMicroseconds());

	signal(SIGKILL, stop);
	signal(SIGSTOP, stop);
	signal(SIGQUIT, stop);
	signal(SIGABRT, stop);
	signal(SIGINT, stop);
}

RAM& E5150::Arch::getRam() { return m_ram; }

void E5150::Arch::startSimulation()
{
#ifndef STOP_AT_END
	sf::Clock clock;
	sf::Time elapsedSinceLastSecond = sf::Time::Zero;
	unsigned int blockCount = 0;
#endif
	unsigned int fdcClock = 0;
	unsigned int currentClock = 0;

	try
	{
		while (Util::_continue)
		{
	#if !defined(STOP_AT_END) && !defined(CLOCK_DEBUG)
			//The simulation simulates blocks of clock instead of raw clock ticks, otherwise the times are too small to be accurately measured.
			//The next block is launch if we have enougth time (we can run at less clock than specified but not more)
			if ((blockCount++)*CLOCK_PER_BLOCKS <= I8284_CLOCKS_PER_SECOND)
			{
				for (unsigned int i = 0; i < CLOCK_PER_BLOCKS; ++i)
				{
	#endif
					++currentClock;
					m_cpu.simulate();
					m_pit.clock();
					
					//Explain this code
					while ((fdcClock+1)*100 <= 167*currentClock)
					{
						++fdcClock;
						m_fdc.clock();
					}
	#if !defined(STOP_AT_END) && !defined(CLOCK_DEBUG)
				}
			}
			//const sf::Time blockDuration = clock.getElapsedTime();

			if (elapsedSinceLastSecond >= sf::seconds(1.f))
			{
				std::cout << "bps: " << blockCount << "   cps: " << blockCount*CLOCK_PER_BLOCKS << std::endl;
				std::cout << "tpb: " << elapsedSinceLastSecond.asMicroseconds()/blockCount << "us\n";
				const float value = 1.f - (float)fdcClock/8000000.f;
				const float acuraccy = (value < 0 ? (-value) : value) * 100.f;
				std::cout << "fdc clock: " << fdcClock << "(" << acuraccy << "%)" << std::endl;
				std::cout << "delay: " << blockCount*CLOCK_PER_BLOCKS / I8284_CLOCKS_PER_SECOND * 100 << "%\n";
				elapsedSinceLastSecond = sf::Time::Zero;
				blockCount = 0;
				fdcClock = 0;
				currentClock = 0;
			}

			while (clock.getElapsedTime() < TIME_PER_BLOCK);

			elapsedSinceLastSecond += clock.restart();
	#else
			if (currentClock >= I8284_CLOCKS_PER_SECOND)
			{
				currentClock = 0;
				fdcClock = 0;
			}
	#endif
		}
	}
	catch (const std::exception& e)
	{ ERROR(e.what()); }
	INFO("Simulation quit !");
}