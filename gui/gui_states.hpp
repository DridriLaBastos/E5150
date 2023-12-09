//
// Created by Adrien COURNAND on 26/12/2022.
//

#ifndef E5150_GUI_STATES_HPP
#define E5150_GUI_STATES_HPP

#include <cstdint>

#include "spdlog_imgui_color_sink.hpp"
#include "debugger/debugger.hpp"

#include "core/8086.hpp"

//Need to use pointer because references are not assignable outside of its declaration
struct DebuggerGuiData
{
	void (*parseLine)(const std::string& cmdLine) = nullptr;

	const CPU* i8086 = nullptr;
	const E5150::I8086::EU::InternalInfos* euWorkingState = nullptr;;
};

struct EmulationGUIState
{
	uint64_t cpuClock, fdcClock,instructionExecutedCount;
	SpdlogImGuiColorSink<std::mutex>* consoleSink;

#ifdef DEBUGGER_ON
	DebuggerGuiData debuggerGuiState;
#endif
};

#define QUOTE(macro) #macro
#define FUNCTION_NAME(name) QUOTE(name)

#define HOT_RELOAD_DRAW_SIGNATURE_RETURN_TYPE void
#define HOT_RELOAD_DRAW_NAME HotReloadDraw
#define HOT_RELOAD_DRAW_SIGNATURE_PARAMETER const EmulationGUIState& emulationGuiState

#define HOT_RELOAD_DRAW_SIGNATURE HOT_RELOAD_DRAW_SIGNATURE_RETURN_TYPE HOT_RELOAD_DRAW_NAME (HOT_RELOAD_DRAW_SIGNATURE_PARAMETER)

#define HOT_RELOAD_DRAW_NAME_STR FUNCTION_NAME(HOT_RELOAD_DRAW_NAME)

#endif //E5150_GUI_STATES_HPP
