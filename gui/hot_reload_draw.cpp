//
// Created by Adrien COURNAND on 24/12/2022.
//

#include "core/pch.hpp"
#include "core/emulation_constants.hpp"
#include "gui_states.hpp"
#include "third-party/imgui/imgui.h"

#include "platform/platform.h"


char ImGuiTextBuffer::EmptyString[1] = { 0 };

using gui_clock = std::chrono::high_resolution_clock;

static constexpr unsigned int MS_PER_UPDATE = 1000;
static constexpr unsigned int EXPECTED_CPU_CLOCK_COUNT = E5150::CPU_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);
static constexpr unsigned int EXPECTED_FDC_CLOCK_COUNT = E5150::FDC_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);

#if 0
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
#endif

enum class DEBUGGER_ENTRY_TYPE {
	COMMAND, DEFAULT
};

struct DebuggerConsoleEntry {
	const std::string line;
	const DEBUGGER_ENTRY_TYPE type;

	DebuggerConsoleEntry(const DEBUGGER_ENTRY_TYPE& t, const std::string& l): line(l), type(t) {}
};

static std::vector<DebuggerConsoleEntry> entries;

static int DebuggerCommandTextEditCallback(ImGuiInputTextCallbackData* data)
{
	switch(data->EventFlag)
	{
		default:
			break;
	}
}

static void DrawDebuggerCommandConsole(const DebuggerGuiData& debuggerGuiData)
{
	if (ImGui::BeginChild("scrolling",ImVec2{150,80},false, ImGuiWindowFlags_HorizontalScrollbar))
	{
		std::for_each(entries.begin(), entries.end(), [](const DebuggerConsoleEntry& entry) {
			if (entry.type == DEBUGGER_ENTRY_TYPE::COMMAND)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0,0.8,0.6,1.0});
			}

			ImGui::TextUnformatted(entry.line.c_str());

			if (entry.type == DEBUGGER_ENTRY_TYPE::COMMAND)
			{
				ImGui::PopStyleColor();
			}
		});
	}
	ImGui::EndChild();

	char cmdBuffer [256];
	memset(cmdBuffer,0,sizeof(cmdBuffer));

	ImGuiInputTextFlags inputTextFlags = 
		ImGuiInputTextFlags_EnterReturnsTrue |
		ImGuiInputTextFlags_EscapeClearsAll |
		ImGuiInputTextFlags_CallbackCompletion |
		ImGuiInputTextFlags_CallbackHistory;

	if(ImGui::InputText("Enter your command", cmdBuffer,IM_ARRAYSIZE(cmdBuffer)-1,inputTextFlags, DebuggerCommandTextEditCallback))
	{
		entries.emplace_back(DEBUGGER_ENTRY_TYPE::COMMAND, std::string(" # ") + std::string(cmdBuffer));
		debuggerGuiData.parseLine(cmdBuffer);
	}
}

static void DrawDebuggerGui(const DebuggerGuiData& debuggerGuiData)
{
	ImGui::Begin("Debugger");

	DrawDebuggerCommandConsole(debuggerGuiData);
	// drawCpuDebugStatus(state);

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

extern "C" DLL_EXPORT void hotReloadDraw(const EmulationGUIState& emulationGuiState, const DebuggerGuiData& debuggerCliFunctions)
{
	drawEmulationConsole(emulationGuiState);
	drawEmulationGui(emulationGuiState);
#ifdef DEBUGGER_ON
	DrawDebuggerGui(debuggerCliFunctions);
#endif
}
