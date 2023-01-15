//
// Created by Adrien COURNAND on 26/12/2022.
//

#ifndef E5150_GUI_STATES_HPP
#define E5150_GUI_STATES_HPP

#include <mutex>
#include <cstdint>

#include "debugger/debugger.hpp"
#include "spdlog_imgui_color_sink.hpp"

struct EmulationGUIState
{
	uint64_t cpuClock, fdcClock,instructionExecutedCount;
	SpdlogImGuiColorSink<std::mutex>* consoleSink;
};

struct DebuggerGUIState
{
	std::mutex* debugConsoleDataAccessMutex;
	E5150::Debugger::GUI::ConsoleEntries* consoleEntries;
};

#endif //E5150_GUI_STATES_HPP
