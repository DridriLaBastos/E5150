#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "pit.hpp"
#include "pic.hpp"
#include "8086.hpp"
#include "ports.hpp"

void PITTest (void)
{
	PORTS port;
	RAM ram;
	CPU cpu (ram,port);
	E5150::PIC pic (port,cpu);
	E5150::PIT pit(port,pic);

	SECTION("Initialization")
	{
		REQUIRE(pit.m_counters.size() == 3);
	}
}

TEST_CASE("Testing PIT")
{
	PITTest();
}