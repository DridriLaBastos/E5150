#include <thread>
#include <signal.h>
#include <SFML/System/Clock.hpp>

#include "arch.hpp"
#include "util.hpp"

using FloatMicroDuration = std::chrono::duration<float, std::micro>;

static constexpr unsigned int CLOCK_PER_BLOCKS = 1500000;
static constexpr unsigned int BASE_CLOCK = 14318181;
static constexpr unsigned int CPU_CLOCK_DIV = 3;
static constexpr unsigned int FDC_CLOCK_DIV = 12;

static constexpr unsigned int TOTAL_CPU_CLOCK = BASE_CLOCK/CPU_CLOCK_DIV;
static constexpr unsigned int TOTAL_FDC_CLOCK = BASE_CLOCK/FDC_CLOCK_DIV;
//static constexpr unsigned int I8284_CLOCKS_PER_SECOND = BASE_CLOCK/3;
static const sf::Time TIME_PER_BLOCK = sf::seconds((float)CLOCK_PER_BLOCKS * 1.f/(float)BASE_CLOCK);

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
	sf::Time timeForAllBlocks = sf::Time::Zero;
	unsigned int blockCount = 0;
	unsigned int fdcClock = 0;
	unsigned int cpuClock = 0;
	unsigned int currentClock = 0;
	unsigned int cpuClockDiv = CPU_CLOCK_DIV;
	unsigned int fdcClockDiv = FDC_CLOCK_DIV;

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
			for (size_t clock = 0; clock < clockToExecute; ++clock)
			{
				--cpuClockDiv;
				--fdcClockDiv;
				if (cpuClockDiv == 0)
				{
					m_cpu.decode();
					#if defined(STOP_AT_END) || defined(CLOCK_DEBUG)
						displayCPUStatusAndWait();
						if (!Util::_continue)
							break;
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
			const sf::Time blockEnd = clock.getElapsedTime();
			++blockCount;
			//~= 70 ns per block
			const sf::Time timeForBlock = blockEnd - blockBegin;
			const unsigned int microsecondsPerBlock = clockToExecute*70/1000;
			const unsigned int microsecondsToWait = microsecondsPerBlock - timeForBlock.asMicroseconds();
			timeForAllBlocks += timeForBlock;

		#if !defined(STOP_AT_END) && !defined(CLOCK_DEBUG)
			std::this_thread::sleep_for(std::chrono::microseconds(microsecondsPerBlock));
		#endif

			if (clock.getElapsedTime() >= sf::seconds(1.f))
			{
			#if not defined(DEBUG_BUILD)
				const float clockAccurency = (float)currentClock/(float)BASE_CLOCK*100.f;
				std::cout << "clock accurency: " << clockAccurency << "%\n";
				std::cout << "blocks: " << blockCount << "/" << BASE_CLOCK/CLOCK_PER_BLOCKS << " "
					<< timeForAllBlocks.asMicroseconds()/blockCount  << "(" << timeForAllBlocks.asMilliseconds()/blockCount
					<< ") us(ms)/block\n\n";
				timeForAllBlocks = sf::Time::Zero;
				clock.restart();
			#endif
				blockCount = 0;
				currentClock = 0;
				cpuClock = 0;
				fdcClock = 0;
			}
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