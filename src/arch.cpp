#include <thread>
#include <signal.h>
#include <SFML/System/Clock.hpp>

#include "arch.hpp"
#include "util.hpp"

using FloatMicroDuration = std::chrono::duration<float, std::micro>;

static constexpr unsigned int CLOCK_PER_BLOCKS = 10000;
static constexpr unsigned int BASE_CLOCK = 14318181;
static constexpr unsigned int CPU_CLOCK_DIV = 3;
static constexpr unsigned int FDC_CLOCK_DIV = 12;
static constexpr unsigned int I8284_CLOCKS_PER_SECOND = BASE_CLOCK/3;
static const sf::Time TIME_PER_BLOCK = sf::seconds((float)CLOCK_PER_BLOCKS/I8284_CLOCKS_PER_SECOND);

bool E5150::Util::_continue = true;
bool E5150::Util::_stop = true;

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

	//irrelevent message if we stop at the end of each instructions
	#ifndef STOP_AT_END
		DEBUG("Duration for {} blocks: {}us", CLOCK_PER_BLOCKS,TIME_PER_BLOCK.asMicroseconds());
	#endif

	signal(SIGKILL, stop);
	signal(SIGSTOP, stop);
	signal(SIGQUIT, stop);
	signal(SIGABRT, stop);
	signal(SIGINT, stop);
}

RAM& E5150::Arch::getRam() { return m_ram; }

void E5150::Arch::startSimulation()
{
	sf::Clock clock;
	sf::Time elapsedSinceLastSecond = sf::Time::Zero;
	unsigned int blockCount = 0;
	unsigned int fdcClock = 0;
	unsigned int cpuClock = 0;
	unsigned int cpuClockDiv = CPU_CLOCK_DIV;
	unsigned int fdcClockDiv = FDC_CLOCK_DIV;

	try
	{
		//TODO: should I go to block of clocks again ?
		while (Util::_continue)
		{
			clock.restart();
			//The simulation simulates blocks of clock instead of raw clock ticks, otherwise the times are too small to be accurately measured.
			//The next block is launch if we have enougth time (we can run at less clock than specified but not more)
			for (size_t currentClock = 0; currentClock < BASE_CLOCK; ++currentClock)
			{
				--cpuClockDiv;
				--fdcClockDiv;
				if (cpuClockDiv == 0)
				{
					m_cpu.decode();
					#if defined(STOP_AT_END) || defined(CLOCK_DEBUG)
						displayCPUStatusAndWait();
					#endif
					m_cpu.exec();
					m_pit.clock();
					++cpuClock;
					cpuClockDiv = CPU_CLOCK_DIV;
				}
				
				if (fdcClockDiv == 0)
				{
					m_fdc.clock();
					++fdcClock;
					fdcClockDiv = FDC_CLOCK_DIV;
				}
			}

		#if !defined(STOP_AT_END) && !defined(CLOCK_DEBUG)
			/*std::cout << "bps: " << blockCount << "   cps: " << blockCount*CLOCK_PER_BLOCKS << std::endl;
			std::cout << "tpb: " << elapsedSinceLastSecond.asMicroseconds()/blockCount << "us\n";
			const float value = 1.f - (float)fdcClock/4000000.f;
			const float acuraccy = (value < 0 ? (-value) : value) * 100.f;
			std::cout << "fdc clock: " << fdcClock << "(" << acuraccy << "%)" << std::endl;
			std::cout << "delay: " << blockCount*CLOCK_PER_BLOCKS / I8284_CLOCKS_PER_SECOND * 100 << "%\n";
			elapsedSinceLastSecond = sf::Time::Zero;
			blockCount = 0;
			//currentClock = 0;
			fdcClock = 0;*/
			std::cout << "cpu clock: " << cpuClock << std::endl;
			std::cout << "fdc clock: " << fdcClock << std::endl << std::endl;
		#endif

			blockCount = 0;
			//currentClock = 0;
			cpuClock = 0;
			fdcClock = 0;

			while (clock.getElapsedTime() <= sf::seconds(1.f));
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
	catch (const std::exception& e)
	{ ERROR(e.what()); }
	INFO("Simulation quit !");
}
void E5150::Arch::wait() const
{
	std::string tmp;
	std::getline(std::cin, tmp);
	if (tmp == "q")
		E5150::Util::_continue = false;
}

void E5150::Arch::displayCPUStatusAndWait() const
{
	if (E5150::Util::_stop)
	{
		m_cpu.printRegisters();
		m_cpu.printFlags();
		m_cpu.printCurrentInstruction();
		wait();
	}
}