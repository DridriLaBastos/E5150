//
// Created by Adrien COURNAND on 26/12/2022.
//

#ifndef E5150_GUI_STATES_HPP
#define E5150_GUI_STATES_HPP

#include <cstdint>

#include "spdlog_imgui_color_sink.hpp"
#include "debugger/cli.hpp"

struct EmulationGUIState
{
	uint64_t cpuClock, fdcClock,instructionExecutedCount;
	SpdlogImGuiColorSink<std::mutex>* consoleSink;
};

struct DebuggerGuiData
{
	void (*parseLine)(const std::string& cmdLine);
};

#endif //E5150_GUI_STATES_HPP
