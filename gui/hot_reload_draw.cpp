//
// Created by Adrien COURNAND on 24/12/2022.
//

#include <inttypes.h>

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
	static size_t previousCallBufferSize = entries.size();
	static size_t entriesHistoriqueIndex = 0;

	if (entries.size() != previousCallBufferSize)
	{
		entriesHistoriqueIndex = 0;
		previousCallBufferSize = entries.size();
	}

	switch(data->EventFlag)
	{
		case ImGuiInputTextFlags_CallbackHistory: {
			if (entries.empty())
				break;

			const DebuggerConsoleEntry& entry = *(entries.end() - 1 - entriesHistoriqueIndex);

			data->DeleteChars(0, data->BufTextLen);
			data->InsertChars(0,entry.line.data());

			int historyDirection = 0;

			if (data->EventKey == ImGuiKey_UpArrow)
				historyDirection = 1;

			if (data->EventKey == ImGuiKey_DownArrow)
				historyDirection = -1;
			entriesHistoriqueIndex = (entriesHistoriqueIndex + historyDirection) % entries.size();
		}
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

			ImGui::TextUnformatted(" #");
			ImGui::SameLine();
			ImGui::TextUnformatted(&(entry.line[0]));

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

	ImGui::Text("(%" PRIu64 ")",debuggerGuiData.i8086->instructionExecutedCount);
	ImGui::SameLine();
	if(ImGui::InputText("Enter your command", cmdBuffer,IM_ARRAYSIZE(cmdBuffer)-1,inputTextFlags, DebuggerCommandTextEditCallback))
	{
		const size_t len = strnlen(cmdBuffer, sizeof(cmdBuffer)) + 1;
		//Ensure that the stored string is a valid null-terminated character without the need to call std::string::c_str
		entries.emplace_back(DEBUGGER_ENTRY_TYPE::COMMAND, std::string(cmdBuffer, len));
		debuggerGuiData.parseLine(cmdBuffer);
	}
}

static void DrawDebuggerCPUStatus(const DebuggerGuiData& debuggerGuiData)
{
	const uint16_t cs = debuggerGuiData.i8086->regs.cs;
	const uint16_t ip = debuggerGuiData.i8086->regs.ip;
	unsigned int ea = (cs << 4) + ip;
	ImGui::Text("Fetching (cs:ip) 0x%04X:0x%04X (0x%05X)",cs,ip,ea);

	const uint16_t ss = debuggerGuiData.i8086->regs.ss;
	const uint16_t sp = debuggerGuiData.i8086->regs.sp;
	ea = (ss << 4) + sp;
	ImGui::Text("Stack    (ss:sp) 0x%04X:0x%04X (0x%05X)", ss,sp,ea);

	ImGui::Separator();

	ImGui::Text("CS 0x%4X   DS 0x%4X   ES 0x%4X   SS 0x%4X",
				debuggerGuiData.i8086->regs.cs,debuggerGuiData.i8086->regs.ds,debuggerGuiData.i8086->regs.es,debuggerGuiData.i8086->regs.ss);
	ImGui::Text("AX 0x%4X   BX 0x%4X   CX 0x%4X   DX 0x%4X",
	            debuggerGuiData.i8086->regs.ax,debuggerGuiData.i8086->regs.bx,debuggerGuiData.i8086->regs.cx,debuggerGuiData.i8086->regs.dx);
	ImGui::Text("SI 0x%4X   DI 0x%4X   BP 0x%4X   SP 0x%4X",
	            debuggerGuiData.i8086->regs.si,debuggerGuiData.i8086->regs.di,debuggerGuiData.i8086->regs.bp,debuggerGuiData.i8086->regs.sp);
	ImGui::Separator();

	const Regs& regs = debuggerGuiData.i8086->regs;
	ImGui::Text("%c %c %c %c %c %c %c %c %c",(regs.flags & CPU::CARRY) ? 'C' : 'c',(regs.flags & CPU::PARRITY) ? 'P' : 'p',
		(regs.flags & CPU::A_CARRY) ? 'A' : 'a', (regs.flags & CPU::ZERRO) ? 'Z' : 'z', (regs.flags & CPU::SIGN) ? 'S' : 's',
		(regs.flags & CPU::TRAP) ? 'T' : 't', (regs.flags & CPU::INTF) ? 'I' : 'i', (regs.flags & CPU::DIR) ? 'D' : 'd',
		(regs.flags & CPU::OVER) ? 'O' : 'o');

}

static void DrawDebuggerGui(const DebuggerGuiData& debuggerGuiData)
{
	ImGui::Begin("Debugger");

	ImGui::BeginChild("Console",ImVec2(ImGui::GetContentRegionAvail().x*.5,0.0),true);
	DrawDebuggerCommandConsole(debuggerGuiData);
	ImGui::EndChild();
	
	ImGui::SameLine();

	ImGui::BeginChild("Reg View", {0,0},true);
	ImGui::PushItemWidth(-FLT_MIN);
	DrawDebuggerCPUStatus(debuggerGuiData);
	ImGui::PopItemWidth();
	ImGui::EndChild();

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
