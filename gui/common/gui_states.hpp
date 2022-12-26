//
// Created by Adrien COURNAND on 26/12/2022.
//

#ifndef E5150_GUI_STATES_HPP
#define E5150_GUI_STATES_HPP

#include <cstdint>

struct EmulationGUIState
{
	uint64_t cpuClock, fdcClock,instructionExecutedCount;
};

struct DebuggerGUIState
{

};

#endif //E5150_GUI_STATES_HPP
