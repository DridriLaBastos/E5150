//
// Created by Adrien COURNAND on 26/12/2022.
//

#ifndef E5150_GUI_STATES_HPP
#define E5150_GUI_STATES_HPP

#include <memory>
#include <cstdint>

#include "core/arch.hpp"
#include "spdlog_imgui_color_sink.hpp"

struct DebuggerGuiState
{
#if 0
	std::string outCmdLine;
	CPU* i8086 = nullptr;
	uint8_t* ramData = nullptr;
	xed_decoded_inst_t* currentlyDecodedInstruction;
#endif
	E5150::Intel8088* cpu;
	std::string outCommandLine;
};

struct InternalState;

struct EmulationGuiState
{
	E5150::Arch::EmulationStat emulationStat;
	SpdlogImGuiColorSink<std::mutex>* consoleSink{};
	InternalState* internalState;

#ifdef DEBUGGER_ON
	DebuggerGuiState debuggerGuiState;
#endif
};

#define QUOTE(macro) #macro
#define FUNCTION_NAME(name) QUOTE(name)

#define HOT_RELOAD_DRAW_SIGNATURE_RETURN_TYPE void
#define HOT_RELOAD_DRAW_NAME HotReloadDraw
#define HOT_RELOAD_DRAW_SIGNATURE_PARAMETER EmulationGuiState& emulationGuiState

#define HOT_RELOAD_DRAW_SIGNATURE HOT_RELOAD_DRAW_SIGNATURE_RETURN_TYPE HOT_RELOAD_DRAW_NAME (HOT_RELOAD_DRAW_SIGNATURE_PARAMETER)

#define HOT_RELOAD_DRAW_NAME_STR FUNCTION_NAME(HOT_RELOAD_DRAW_NAME)

#endif //E5150_GUI_STATES_HPP
