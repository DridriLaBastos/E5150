#include <chrono>
#include <unistd.h>

#include "arch.hpp"

using Clock = std::chrono::system_clock;
using FloatMicroDuration = std::chrono::duration<float, std::micro>;

static constexpr float CPU_CLOCK_PER_SECOND = 4770000.f;
static constexpr FloatMicroDuration CPU_CLOCK_DURATION(std::micro::den / CPU_CLOCK_PER_SECOND);

static bool _continue = true;

static void stop(const int)
{
	_continue = false;
	std::cout << "Simulation stoped !" << std::endl;
#ifdef STOP_AT_END
	std::cout << "Press <enter> to quit !" << std::endl;
#endif
}

E5150::Arch::Arch(): m_ram(), m_ports(), m_cpu(m_ram, m_ports, *this), m_pic(m_ports, m_cpu), m_pit(m_ports, m_pic)
{
	std::cout << "Welcome to E5150, the emulator of an IBM PC 5150" << std::endl;
	std::cout << "This program use the library Intel XED to decode the instructions" << std::endl;
	std::cout << "This library is accessible at : https://intelxed.github.io" << std::endl;
	std::cout << "xed version : " << xed_get_version() << std::endl;

	signal(SIGKILL, stop);
	signal(SIGSTOP, stop);
	signal(SIGQUIT, stop);
	signal(SIGABRT, stop);
	signal(SIGINT, stop);
}

void E5150::Arch::stopSimulation() const { _continue = false; }

RAM& E5150::Arch::getRam() { return m_ram; }

void E5150::Arch::startSimulation()
{
#ifndef STOP_AT_END
	FloatMicroDuration elapsed;
#endif

	while (_continue)
	{
#ifndef STOP_AT_END
		Clock::time_point start = Clock::now();
		while (elapsed >= CPU_CLOCK_DURATION)
		{
#endif
			m_cpu.simulate();
			m_pit.clock();
#ifndef STOP_AT_END
			elapsed -= CPU_CLOCK_DURATION;
		}

		FloatMicroDuration duration = std::chrono::duration_cast<FloatMicroDuration>(Clock::now() - start);
		elapsed += duration;
#endif
	}
}