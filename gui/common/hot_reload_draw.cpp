//
// Created by Adrien COURNAND on 24/12/2022.
//

#include "core/pch.hpp"
#include "core/emulation_constants.hpp"
#include "gui_states.hpp"
#include "third-party/imgui/imgui.h"
#include "spdlog_imgui_color_sink.hpp"
#include "gui_states.hpp"
#include "debugger/debugger.hpp"
#include "platform/platform.h"

char ImGuiTextBuffer::EmptyString[1] = { 0 };

using gui_clock = std::chrono::high_resolution_clock;

static constexpr unsigned int MS_PER_UPDATE = 1000;
static constexpr unsigned int EXPECTED_CPU_CLOCK_COUNT = E5150::CPU_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);
static constexpr unsigned int EXPECTED_FDC_CLOCK_COUNT = E5150::FDC_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);

static void drawDebuggerLoggingConsole(E5150::DEBUGGER::GUI::State* const state)
{
	static constexpr size_t DEBUGGER_COMMAND_BUFFER_SIZE = 256;
	static char debuggerCommandInputBuffer [DEBUGGER_COMMAND_BUFFER_SIZE];

	state->debugConsoleMutex->lock();
	size_t s = state->debugConsoleEntries->size();
	state->debugConsoleMutex->unlock();

	{
		const ImGuiWindowFlags consoleFlags = ImGuiWindowFlags_HorizontalScrollbar;
		ImGui::BeginChild("Console",{0,ImGui::GetContentRegionAvail().y * 0.66f}, false, consoleFlags);
		for (size_t i = 0; i < s; ++i) {
			bool hasColor = true;
			//Since a push back can potentialy invalidate data inside a vector, we need to lock it during the whole
			//iteration
			state->debugConsoleMutex->lock();

			const auto &entry = (*(state->debugConsoleEntries))[i];

			switch (entry.type) {
				case E5150::DEBUGGER::GUI::CONSOLE_ENTRY_TYPE::COMMAND:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f));
					break;
				case E5150::DEBUGGER::GUI::CONSOLE_ENTRY_TYPE::DEBUGGER_STDERR:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.1f, 0.2f, 1.0f));
					break;
				default:
					hasColor = false;
					break;
			}

			ImGui::Text("%s%s", entry.prefix.c_str(), entry.str.c_str());
			if (hasColor) { ImGui::PopStyleColor(); }
			s = state->debugConsoleEntries->size();
			state->debugConsoleMutex->unlock();
		}
		ImGui::EndChild();
	}
	ImGui::Text("(%llu)",state->i8086->instructionExecutedCount);

	ImGui::SameLine();
	if (ImGui::InputText("Debugger command",debuggerCommandInputBuffer,DEBUGGER_COMMAND_BUFFER_SIZE,ImGuiInputTextFlags_EnterReturnsTrue))
	{
		state->debugConsoleEntries->emplace_back(E5150::DEBUGGER::GUI::CONSOLE_ENTRY_TYPE::COMMAND, debuggerCommandInputBuffer, " # ");
	}
}

static void drawCpuDebugStatus(E5150::DEBUGGER::GUI::State* const state)
{
	ImGui::TextUnformatted("Instruction placeholder");
	ImGui::Text("CS 0x%4X   ", state->i8086->regs.cs);
	ImGui::SameLine();
	ImGui::Text("DS 0x%4X   ", state->i8086->regs.ds);
	ImGui::SameLine();
	ImGui::Text("ES 0x%4X   ", state->i8086->regs.es);
	ImGui::SameLine();
	ImGui::Text("SS 0x%4X   ", state->i8086->regs.ss);
	ImGui::SameLine();
	ImGui::Text("IP 0x%4X   ", state->i8086->regs.ip);

	ImGui::Text("AX 0x%4X   ", state->i8086->regs.ax);
	ImGui::SameLine();
	ImGui::Text("BX 0x%4X   ", state->i8086->regs.bx);
	ImGui::SameLine();
	ImGui::Text("CX 0x%4X   ", state->i8086->regs.cx);
	ImGui::SameLine();
	ImGui::Text("DX 0x%4X   ", state->i8086->regs.dx);

	ImGui::Text("DI 0x%4X   ", state->i8086->regs.di);
	ImGui::SameLine();
	ImGui::Text("SI 0x%4X   ", state->i8086->regs.si);
	ImGui::SameLine();
	ImGui::Text("BP 0x%4X   ", state->i8086->regs.bp);
	ImGui::SameLine();
	ImGui::Text("SP 0x%4X   ", state->i8086->regs.sp);
}

static void drawDebuggerGui(E5150::DEBUGGER::GUI::State* const state)
{
	ImGui::Begin("Debugger");

	drawDebuggerLoggingConsole(state);
	drawCpuDebugStatus(state);

	ImGui::End();
}

static void drawEmulationConsole(const EmulationGUIState& state)
{ state.consoleSink->imguiDraw(); }

static void drawEmulationGui(const EmulationGUIState& state)
{
	static unsigned int instructionExecuted = 0;
	static uint64_t lastFrameCPUClockCount = 0;
	static unsigned int lastFrameCPUInstructionCount = 0;
	static uint64_t lastFrameFDCClockCount = 0;
	static unsigned int CPUClockDelta = 0;
	static unsigned int FDCClockDelta = 0;
	static unsigned int CPUClockAccuracy = 0;
	static unsigned int FDCClockAccuracy = 0;
	static auto lastUpdateTime = gui_clock::now();

	const auto now = gui_clock::now();
	const auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime);
	if (timeSinceLastUpdate >= std::chrono::milliseconds(MS_PER_UPDATE))
	{
		instructionExecuted = state.instructionExecutedCount - lastFrameCPUInstructionCount;

		const uint64_t currentCPUClockCount = state.cpuClock;
		const uint64_t currentFDCClockCount = state.fdcClock;

		CPUClockDelta = currentCPUClockCount - lastFrameCPUClockCount;
		FDCClockDelta = currentFDCClockCount - lastFrameFDCClockCount;

		CPUClockAccuracy = CPUClockDelta * 100 / EXPECTED_CPU_CLOCK_COUNT;
		FDCClockAccuracy = FDCClockDelta * 100 / EXPECTED_FDC_CLOCK_COUNT;

		lastFrameCPUInstructionCount = state.instructionExecutedCount;
		lastFrameCPUClockCount = currentCPUClockCount;
		lastFrameFDCClockCount = currentFDCClockCount;

		lastUpdateTime = now;
	}
	ImGui::Begin("Emulation Statistics");

	ImGui::Text("CPU clock executed : %6d / %6d", CPUClockDelta, EXPECTED_CPU_CLOCK_COUNT);
	ImGui::Text("FDC clock executed : %6d / %6d", FDCClockDelta, EXPECTED_FDC_CLOCK_COUNT);
	ImGui::Text("Clock accuracy cpu : %d%%  fdc %d%%", CPUClockAccuracy, FDCClockAccuracy);
	ImGui::Text("Instruction executed : %.3fM/s ( %4.3fM)",instructionExecuted / 1e6, state.instructionExecutedCount / 1e6);
	// ImGui::TextUnformatted("Bonjour");

	ImGui::End();
}

extern "C" DLL_EXPORT void hotReloadDraw(const EmulationGUIState& emulationGuiState, E5150::DEBUGGER::GUI::State* const debuggerGUIState)
{
	drawEmulationConsole(emulationGuiState);
	drawEmulationGui(emulationGuiState);

#ifdef DEBUGGER_ON
	drawDebuggerGui(debuggerGUIState);
#endif
}
