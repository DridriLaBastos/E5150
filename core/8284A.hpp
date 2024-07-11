//
// Created by Adrien COURNAND on 23/06/2024.
//

#ifndef E5150_8284A_HPP
#define E5150_8284A_HPP

namespace E5150
{
	class Intel8284A
	{
	public:
		static constexpr unsigned int FREQUENCY_DIVIDER = 3;
		static constexpr unsigned int BASE_FREQUENCY_HZ = 14318180;
		static constexpr unsigned int CPU_FREQUENCY_HZ = BASE_FREQUENCY_HZ / FREQUENCY_DIVIDER;
	};
}

#endif //E5150_8284A_HPP
