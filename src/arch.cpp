#include <thread>
#include <signal.h>
#include <SFML/System/Clock.hpp>

#include "arch.hpp"
#include "util.hpp"

using Clock = std::chrono::system_clock;
using FloatMicroDuration = std::chrono::duration<float, std::micro>;

constexpr unsigned int CLOCK_PER_BLOCKS = 10000;
static constexpr float I8284_CLOCKS_PER_SECOND = 4770000.f;
static const sf::Time TIME_PER_BLOCK = sf::seconds((float)CLOCK_PER_BLOCKS/I8284_CLOCKS_PER_SECOND);

bool E5150::Util::_continue = true;

static void stop(const int signum)
{
	E5150::Util::_continue = false;
	INFO("Simulation stopped by signal {}", signum);
}

E5150::Arch::Arch(): m_ram(), m_cpu(m_ram, m_ports), m_pic(m_ports, m_cpu), m_pit(m_ports, m_pic), m_ppi(m_ports)
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

	try
	{
		while (Util::_continue)
		{
	#if !defined(STOP_AT_END) && !defined(CLOCK_DEBUG)
			//The simulation simulates blocks of clock instead of raw clock ticks, otherwise the times are too small to be accurately measured.
			//The next block is launch if we have enougth time (we can run at less clock than specified but not more)
			if ((blockCount+1)*CLOCK_PER_BLOCKS <= I8284_CLOCKS_PER_SECOND)
			{
				for (unsigned int i = 0; i < CLOCK_PER_BLOCKS; ++i)
				{
	#endif
					m_cpu.simulate();
					m_pit.clock();
	#if !defined(STOP_AT_END) && !defined(CLOCK_DEBUG)
				}
				++blockCount;
			}
			const sf::Time blockDuration = clock.getElapsedTime();

			if (elapsedSinceLastSecond >= sf::seconds(1.f))
			{
				std::cout << "bps: " << blockCount << "   cps: " << blockCount*CLOCK_PER_BLOCKS << std::endl;
				std::cout << "tpb: " << elapsedSinceLastSecond.asMicroseconds()/blockCount << "us\n";
				std::cout << "delay: " << blockCount*CLOCK_PER_BLOCKS / I8284_CLOCKS_PER_SECOND * 100 << "%\n";
				blockCount = 0;
				elapsedSinceLastSecond = sf::Time::Zero;
			}

			while (clock.getElapsedTime() < TIME_PER_BLOCK);

			elapsedSinceLastSecond += clock.restart();
	#endif
		}
	}
	catch (const std::exception& e)
	{ ERROR(e.what()); }
	INFO("Simulation quit !");
}