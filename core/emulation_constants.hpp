//
// Created by Adrien COURNAND on 26/12/2022.
//

#ifndef E5150_EMULATION_DATA_HPP
#define E5150_EMULATION_DATA_HPP

namespace E5150
{
	constexpr unsigned int CPU_BASE_CLOCK = 14318181;
	constexpr unsigned int FDC_BASE_CLOCK =  4000000;
	constexpr unsigned int CLOCK_PER_BLOCK = 1500000;
	constexpr unsigned int NS_PER_CLOCK = 1.f/CPU_BASE_CLOCK*1e9+.5f;
	constexpr unsigned int BLOCK_PER_SECOND = CPU_BASE_CLOCK / CLOCK_PER_BLOCK;
}

#endif //E5150_EMULATION_DATA_HPP
