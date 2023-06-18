//
// Created by Adrien COURNAND on 26/12/2022.
//

#ifndef E5150_GUI_STATES_HPP
#define E5150_GUI_STATES_HPP

#include <cstdint>

#include "spdlog_imgui_color_sink.hpp"
#include "debugger/debugger.hpp"

#include "core/8086.hpp"

struct EmulationGUIState
{
	uint64_t cpuClock, fdcClock,instructionExecutedCount;
	SpdlogImGuiColorSink<std::mutex>* consoleSink;
};

//Need to use pointer because references are not assignable outside of its declaration
struct DebuggerGuiData
{
	void (*parseLine)(const std::string& cmdLine) = nullptr;

	const CPU* i8086 = nullptr;
	const E5150::I8086::EU::InternalInfos* euWorkingState = nullptr;;
};

#endif //E5150_GUI_STATES_HPP
