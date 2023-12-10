//
// Created by Adrien COURNAND on 24/12/2022.
//
//TODO: WHY DO I HAVE LINK ERROR WHEN COMPILING XED IN STATIC MODE ?
#include <cinttypes>

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

	//https://github.com/ocornut/imgui/blob/b9db5c566bbafa3462f1a526032ca2971742db17/imgui_widgets.cpp#L4003
	//The value returned should be different than 0 (I guess that's what the line says ?)
	return !0;
}

static void DrawDebuggerCommandConsole(DebuggerGuiState& debuggerGuiState)
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

	ImGui::Text("(%" PRIu64 ")",debuggerGuiState.instructionExecutedCount);
	ImGui::SameLine();
	if(ImGui::InputText("Enter your command", cmdBuffer,IM_ARRAYSIZE(cmdBuffer)-1,inputTextFlags, DebuggerCommandTextEditCallback))
	{
		const size_t len = strnlen(cmdBuffer, sizeof(cmdBuffer)) + 1;
		//Ensure that the stored string is a valid null-terminated character without the need to call std::string::c_str
		entries.emplace_back(DEBUGGER_ENTRY_TYPE::COMMAND, std::string(cmdBuffer, len));
		debuggerGuiState.outCmdLine = cmdBuffer;
	}

	ImGui::Separator();
	if (ImGui::Button("|->I"))
	{
		debuggerGuiState.outCmdLine = "step --instruction 1";
	}

	ImGui::SameLine();
	if (ImGui::Button("|->C"))
	{
		debuggerGuiState.outCmdLine = "step --clock 1";
	}

	ImGui::SameLine();
	if (ImGui::Button("->>"))
	{
		debuggerGuiState.outCmdLine = "continue";
	}
}

static void DrawCurrentInstruction(const DebuggerGuiState& data)
{
	const xed_inst_t* inst = xed_decoded_inst_inst(data.currenltyDecodedInstruction);

	if (!inst)
		return;

	ImGui::Text("%s : length = %d",xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(data.currenltyDecodedInstruction)),xed_decoded_inst_get_length(data.currenltyDecodedInstruction));
	ImGui::Text("%s",xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(data.currenltyDecodedInstruction))); ImGui::SameLine();
	unsigned int realOperandPos = 0;
	bool foundPtr = false;

	for (unsigned int i = 0; i < xed_decoded_inst_noperands(data.currenltyDecodedInstruction); ++i)
	{
		const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, i));
		const xed_operand_visibility_enum_t op_vis = xed_operand_operand_visibility(xed_inst_operand(inst, i));

		if (op_vis == XED_OPVIS_EXPLICIT)
		{
			if (foundPtr)
			{
				ImGui::TextUnformatted(":"); ImGui::SameLine();
				foundPtr = false;
			}
			else
			{
				if (realOperandPos > 0)
					ImGui::TextUnformatted(", "); ImGui::SameLine();
			}

			switch (op_name)
			{
			case XED_OPERAND_RELBR:
				ImGui::Text("%d",(xed_decoded_inst_get_branch_displacement(data.currenltyDecodedInstruction) & 0xFFFF)); ImGui::SameLine();
				break;

			case XED_OPERAND_PTR:
				ImGui::Text("0x%X",(xed_decoded_inst_get_branch_displacement(data.currenltyDecodedInstruction) & 0xFFFF)); ImGui::SameLine();
				foundPtr = true;
				break;

			case XED_OPERAND_REG0:
			case XED_OPERAND_REG1:
			case XED_OPERAND_REG2:
				ImGui::Text("%s",xed_reg_enum_t2str(xed_decoded_inst_get_reg(data.currenltyDecodedInstruction, op_name))); ImGui::SameLine();
				break;

			case XED_OPERAND_IMM0:
			case XED_OPERAND_IMM1:
			ImGui::Text("0x%" PRIu64 ,(xed_decoded_inst_get_unsigned_immediate(data.currenltyDecodedInstruction) & 0xFFFF)); ImGui::SameLine();
				break;

			//Displaying memory operand with format SEG:[[BASE +] [INDEX +] DISPLACEMENT ]
			case XED_OPERAND_MEM0:
			{
				const xed_reg_enum_t baseReg = xed_decoded_inst_get_base_reg(data.currenltyDecodedInstruction, 0);
				const xed_reg_enum_t indexReg = xed_decoded_inst_get_index_reg(data.currenltyDecodedInstruction, 0);
				const int64_t memDisplacement = xed_decoded_inst_get_memory_displacement(data.currenltyDecodedInstruction,0);
				ImGui::Text("%s %s:[",
					((xed_decoded_inst_get_memory_operand_length(data.currenltyDecodedInstruction, 0) == 1) ? "BYTE" : "WORD"),
					xed_reg_enum_t2str(xed_decoded_inst_get_seg_reg(data.currenltyDecodedInstruction, 0))); ImGui::SameLine();

				if (baseReg != XED_REG_INVALID)
					ImGui::Text("%s",xed_reg_enum_t2str(baseReg)); ImGui::SameLine();
				
				if (indexReg != XED_REG_INVALID)
				{
					if (baseReg != XED_REG_INVALID)
					{ ImGui::TextUnformatted("+"); ImGui::SameLine(); }

					ImGui::Text("%s",xed_reg_enum_t2str(indexReg));
				}

				if ((indexReg != XED_REG_INVALID) || (baseReg != XED_REG_INVALID))
				{
					if (memDisplacement != 0)
					{
						if (memDisplacement > 0)
							ImGui::Text(" + %" PRIi64 , memDisplacement);
						else
							ImGui::Text(" - %" PRIi64 , -memDisplacement);
						ImGui::SameLine();
					}
				}
				else
				{ ImGui::Text("%" PRIi64 ,memDisplacement); ImGui::SameLine(); }

				ImGui::TextUnformatted("]");
				ImGui::SameLine();
			}	break;

			default:
				break;
			}

			++realOperandPos;
		}
	}
		ImGui::Text("(iform: %s)",xed_iform_enum_t2str(xed_decoded_inst_get_iform_enum(data.currenltyDecodedInstruction)));
		ImGui::SameLine();
		ImGui::Text(" (%" PRIu64 ")",data.instructionExecutedCount);
}

static void DrawDebuggerCPUStatus(const DebuggerGuiState& debuggerGuiState)
{
	DrawCurrentInstruction(debuggerGuiState);
#if 0
	const uint16_t cs = debuggerGuiState.i8086->regs.cs;
	const uint16_t ip = debuggerGuiState.i8086->regs.ip;
	unsigned int ea = (cs << 4) + ip;
	ImGui::Text("Fetching (cs:ip) 0x%04X:0x%04X (0x%05X)",cs,ip,ea);
	DrawCurrentInstruction(debuggerGuiState);

	ImGui::Separator();

	const uint16_t ss = debuggerGuiState.i8086->regs.ss;
	const uint16_t sp = debuggerGuiState.i8086->regs.sp;
	ea = (ss << 4) + sp;
	ImGui::Text("Stack    (ss:sp) 0x%04X:0x%04X (0x%05X)", ss,sp,ea);

	ImGui::Separator();

	ImGui::Text("CS 0x%4X   DS 0x%4X   ES 0x%4X   SS 0x%4X",
				debuggerGuiState.i8086->regs.cs,debuggerGuiState.i8086->regs.ds,debuggerGuiState.i8086->regs.es,debuggerGuiState.i8086->regs.ss);
	ImGui::Text("AX 0x%4X   BX 0x%4X   CX 0x%4X   DX 0x%4X",
	            debuggerGuiState.i8086->regs.ax,debuggerGuiState.i8086->regs.bx,debuggerGuiState.i8086->regs.cx,debuggerGuiState.i8086->regs.dx);
	ImGui::Text("SI 0x%4X   DI 0x%4X   BP 0x%4X   SP 0x%4X",
	            debuggerGuiState.i8086->regs.si,debuggerGuiState.i8086->regs.di,debuggerGuiState.i8086->regs.bp,debuggerGuiState.i8086->regs.sp);
	ImGui::Separator();

	const Regs& regs = debuggerGuiState.i8086->regs;
	ImGui::Text("%c %c %c %c %c %c %c %c %c",(regs.flags & CPU::CARRY) ? 'C' : 'c',(regs.flags & CPU::PARRITY) ? 'P' : 'p',
		(regs.flags & CPU::A_CARRY) ? 'A' : 'a', (regs.flags & CPU::ZERRO) ? 'Z' : 'z', (regs.flags & CPU::SIGN) ? 'S' : 's',
		(regs.flags & CPU::TRAP) ? 'T' : 't', (regs.flags & CPU::INTF) ? 'I' : 'i', (regs.flags & CPU::DIR) ? 'D' : 'd',
		(regs.flags & CPU::OVER) ? 'O' : 'o');
#endif
}

static void DrawDebuggerGui(DebuggerGuiState& debuggerGuiState)
{
	ImGui::Begin("Debugger");

	ImGui::BeginChild("Console",ImVec2(ImGui::GetContentRegionAvail().x*.5,0.0),true);
	DrawDebuggerCommandConsole(debuggerGuiState);
	ImGui::EndChild();
	
	ImGui::SameLine();

	ImGui::BeginChild("Reg View", {0,0},true);
	ImGui::PushItemWidth(-FLT_MIN);
	DrawDebuggerCPUStatus(debuggerGuiState);
	ImGui::PopItemWidth();
	ImGui::EndChild();

	ImGui::End();
}

static void DrawEmulationConsole(const EmulationGuiState& state)
{ state.consoleSink->imguiDraw(); }

static void DrawEmulationGui(const EmulationGuiState& state)
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

	ImGui::End();
}

extern "C" DLL_EXPORT HOT_RELOAD_DRAW_SIGNATURE
{
	DrawEmulationConsole(emulationGuiState);
	DrawEmulationGui(emulationGuiState);
#ifdef DEBUGGER_ON
	DrawDebuggerGui(emulationGuiState.debuggerGuiState);
#endif
}
