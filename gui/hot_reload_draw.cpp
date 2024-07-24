//
// Created by Adrien COURNAND on 24/12/2022.
//
//TODO: WHY DO I HAVE LINK ERROR WHEN COMPILING XED IN STATIC MODE ?

#include "core/8284A.hpp"
#include "gui_states.hpp"
#include "third-party/imgui/imgui.h"

#include "platform/platform.h"

using HighResolutionClock = std::chrono::high_resolution_clock;

enum class DEBUGGER_ENTRY_TYPE {
	COMMAND, DEFAULT
};

struct DebuggerConsoleEntry {
	const std::string line;
	const DEBUGGER_ENTRY_TYPE type;

	DebuggerConsoleEntry(const DEBUGGER_ENTRY_TYPE& t, std::string l): line(std::move(l)), type(t) {}
};

struct InternalState
{
	std::vector<DebuggerConsoleEntry> entries;
	E5150::Arch::EmulationStat lastFrameCpuStat;
	HighResolutionClock::time_point lastFrameTime;
};

char ImGuiTextBuffer::EmptyString[1] = { 0 };

static int DebuggerCommandTextEditCallback(ImGuiInputTextCallbackData* data)
{
	static size_t previousCallBufferSize = 0;
	static size_t entriesHistoriqueIndex = 0;
	std::vector<DebuggerConsoleEntry>& entries = ((InternalState*)data->UserData)->entries;

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
	//The value returned should be different from 0 (I guess that's what the line says ?)
	return 1;
}

#if 0
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

	ImGui::Text("(%" PRIu64 ")",debuggerGuiState.i8086->instructionExecutedCount);
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
	const xed_inst_t* inst = xed_decoded_inst_inst(data.currentlyDecodedInstruction);

	if (!inst)
		return;

	ImGui::Text("%s",xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(data.currentlyDecodedInstruction))); ImGui::SameLine();
	unsigned int realOperandPos = 0;
	bool foundPtr = false;

	for (unsigned int i = 0; i < xed_decoded_inst_noperands(data.currentlyDecodedInstruction); ++i)
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
				{
					ImGui::TextUnformatted(", ");
					ImGui::SameLine();
				}
			}

			switch (op_name)
			{
			case XED_OPERAND_RELBR:
				ImGui::Text("%d",(xed_decoded_inst_get_branch_displacement(data.currentlyDecodedInstruction) & 0xFFFF)); ImGui::SameLine();
				break;

			case XED_OPERAND_PTR:
				ImGui::Text("0x%X",(xed_decoded_inst_get_branch_displacement(data.currentlyDecodedInstruction) & 0xFFFF)); ImGui::SameLine();
				foundPtr = true;
				break;

			case XED_OPERAND_REG0:
			case XED_OPERAND_REG1:
			case XED_OPERAND_REG2:
				ImGui::Text("%s",xed_reg_enum_t2str(xed_decoded_inst_get_reg(data.currentlyDecodedInstruction, op_name))); ImGui::SameLine();
				break;

			case XED_OPERAND_IMM0:
			case XED_OPERAND_IMM1:
			ImGui::Text("0x%" PRIu64 ,(xed_decoded_inst_get_unsigned_immediate(data.currentlyDecodedInstruction) & 0xFFFF)); ImGui::SameLine();
				break;

			//Displaying memory operand with format SEG:[[BASE +] [INDEX +] DISPLACEMENT ]
			case XED_OPERAND_MEM0:
			{
				const xed_reg_enum_t baseReg = xed_decoded_inst_get_base_reg(data.currentlyDecodedInstruction, 0);
				const xed_reg_enum_t indexReg = xed_decoded_inst_get_index_reg(data.currentlyDecodedInstruction, 0);
				const int64_t memDisplacement = xed_decoded_inst_get_memory_displacement(data.currentlyDecodedInstruction,0);
				ImGui::Text("%s %s:[",
					((xed_decoded_inst_get_memory_operand_length(data.currentlyDecodedInstruction, 0) == 1) ? "BYTE" : "WORD"),
					xed_reg_enum_t2str(xed_decoded_inst_get_seg_reg(data.currentlyDecodedInstruction, 0))); ImGui::SameLine();

				if (baseReg != XED_REG_INVALID)
				{
					ImGui::Text("%s",xed_reg_enum_t2str(baseReg));
					ImGui::SameLine();
				}
				
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

	ImGui::Text("[%d]",xed_decoded_inst_get_length(data.currentlyDecodedInstruction));
	ImGui::SameLine();
	ImGui::Text("(iform: %s)",xed_iform_enum_t2str(xed_decoded_inst_get_iform_enum(data.currentlyDecodedInstruction)));
	ImGui::SameLine();
	ImGui::Text(" (%" PRIu64 ")",data.i8086->instructionExecutedCount);
}
#endif

static void DrawEmulationConsole(const EmulationGuiState& state)
{ state.consoleSink->imguiDraw(); }

static void DrawEmulationGui(const EmulationGuiState& state)
{
	ImGui::Begin("Emulation Statistics");

	ImGui::Text("CPU clock executed : %6lld / %6u", state.emulationStat.cpuClock, E5150::Intel8284A::CPU_FREQUENCY_HZ);
#if 0
	ImGui::Text("FDC clock executed : %6d / %6d", FDCClockDelta, EXPECTED_FDC_CLOCK_COUNT);
#endif

	const float cpuClockAccuracy = (float)state.emulationStat.cpuClock / E5150::Intel8284A::CPU_FREQUENCY_HZ*100.f;
	ImGui::Text("Clock accuracy:\n\t- cpu : %.2f%%",cpuClockAccuracy);

	ImGui::End();
}

#ifdef DEBUGGER_ON
static void DrawCpuState(const E5150::Intel8088& cpu)
{
	switch (cpu.mode)
	{
		case E5150::Intel8088::ERunningMode::INIT_SEQUENCE:
			ImGui::Text("Init sequence %d", cpu.clock);
			break;

		default:
			break;
	}
}

static void DrawCpuBIUState(const E5150::Intel8088& cpu)
{
	char* stateStr;
	unsigned int clockCount;

	switch (cpu.biuMode)
	{
		case E5150::Intel8088::EBIURunningMode::FETCH_MEMORY:
			stateStr = "Fetch Memory";
			clockCount = cpu.biuClockCountDown;
			break;

		case E5150::Intel8088::EBIURunningMode::WAIT_ROOM_IN_QUEUE:
			stateStr = "Instruction stream full";
			clockCount = 0;
			break;

		default:
			break;
	}

	ImGui::Text("BIU Mode : %s (%d) [%zu]",stateStr,clockCount,cpu.instructionStreamQueueIndex);

	for (size_t i = 0; i < E5150::Intel8088::INSTRUCTION_STREAM_QUEUE_LENGTH; i += 1)
	{
		bool styleColorPushed = false;
		if (i < cpu.instructionStreamQueueIndex)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImColor(.1f,.8f,.3f).Value);
			styleColorPushed = true;
		}
		else if (i == cpu.instructionStreamQueueIndex)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImColor(.8f,.1f,.3f).Value);
			styleColorPushed = true;
		}
		ImGui::Text( "0x%2X", cpu.instructionStreamQueue[i]);
		ImGui::SameLine();

		if (styleColorPushed)
		{
			ImGui::PopStyleColor();
		}
	}
}

static void DrawCpuEUState(const E5150::Intel8088& cpu)
{
	switch (cpu.euMode)
	{
		case E5150::Intel8088::EEURunningMode::WAIT_INSTRUCTION:
			ImGui::TextUnformatted("EU Mode 'Wait instruction'");
			break;

		case E5150::Intel8088::EEURunningMode::EXECUTE_INSTRUCTION:
			ImGui::TextUnformatted("EU Mode 'Execute instruction'");
			break;

		default:
			break;
	}
}

static void DrawCpuWorkingState(const E5150::Intel8088& cpu)
{
	DrawCpuState(cpu);

	ImGui::BeginGroup();
	DrawCpuBIUState(cpu);
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	DrawCpuEUState(cpu);
	ImGui::EndGroup();
}

static void DrawRegisterView(const E5150::Intel8088::Regs& regs)
{
	const uint16_t cs = regs.cs;
	const uint16_t ip = regs.ip;
	unsigned int ea = (cs << 4) + ip;
	ImGui::Text("Fetching (cs:ip) 0x%04X:0x%04X (0x%05X)",cs,ip,ea);

	ImGui::Separator();

	const uint16_t ss = regs.ss;
	const uint16_t sp = regs.sp;
	ea = (ss << 4) + sp;
	ImGui::Text("Stack    (ss:sp) 0x%04X:0x%04X (0x%05X)", ss,sp,ea);

	ImGui::Separator();

	ImGui::Text("CS 0x%4X   DS 0x%4X   ES 0x%4X   SS 0x%4X",
	            regs.cs,regs.ds,regs.es,regs.ss);
	ImGui::Text("AX 0x%4X   BX 0x%4X   CX 0x%4X   DX 0x%4X",
	            regs.ax,regs.bx,regs.cx,regs.dx);
	ImGui::Text("SI 0x%4X   DI 0x%4X   BP 0x%4X   SP 0x%4X",
	            regs.si,regs.di,regs.bp,regs.sp);
	ImGui::Separator();

	const char flagChars[] = { 'C', 'P', 'A', 'Z', 'S', 'T', 'I', 'D', 'O' };

	for (int i = 0; i < 9; i += 1)
	{
		const bool flagSet = regs.flags & (1 << i);
		char flagChar = flagChars[i];

		const ImColor textColor = flagSet ? ImColor{.1f,.9f,.2f} : ImColor{.9f,.1f,.3f};

		if (!flagSet)
		{
			flagChar = flagChar - 'A' + 'a';
		}

		ImGui::TextColored(textColor,"%c",flagChar);
		ImGui::SameLine();
	}
}

static void DrawCpuInternalState(const E5150::Intel8088& cpu)
{
	DrawCpuWorkingState(cpu);
	DrawRegisterView(cpu.regs);
}

static void DrawDebuggerCPUStatus(const EmulationGuiState& emulationGuiState)
{
#if 0
	DrawCurrentInstruction(debuggerGuiState);
#endif
	DrawCpuInternalState(*emulationGuiState.debuggerGuiState.cpu);
}

static void SendCommandToDebugger(const char* cmd, EmulationGuiState& emulationGuiState)
{
	emulationGuiState.debuggerGuiState.outCommandLine = cmd;
	emulationGuiState.internalState->entries.emplace_back(DEBUGGER_ENTRY_TYPE::COMMAND,
	                                                      emulationGuiState.debuggerGuiState.outCommandLine);
}

static void DrawDebuggerConsole(EmulationGuiState& emulationGuiState)
{
	//In case I forgot to remove it : this is just an anchor for ctrl F
	//DrawDebuggerCommandConsole

	if (ImGui::BeginChild("scrolling",ImVec2(0,0),false, ImGuiWindowFlags_HorizontalScrollbar))
	{
		for (DebuggerConsoleEntry& entry : emulationGuiState.internalState->entries)
		{
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
		}
		ImGui::EndChild();

		char cmdBuffer[256];
		memset(cmdBuffer,0,sizeof(cmdBuffer));

		ImGuiInputTextFlags inputTextFlags =
				ImGuiInputTextFlags_EnterReturnsTrue |
				ImGuiInputTextFlags_EscapeClearsAll |
				ImGuiInputTextFlags_CallbackCompletion |
				ImGuiInputTextFlags_CallbackHistory;

		ImGui::Text("(%" PRIu64 ")",emulationGuiState.emulationStat.instructionExecutedCount);
		ImGui::SameLine();
		if(ImGui::InputText("Enter your command", cmdBuffer,IM_ARRAYSIZE(cmdBuffer)-1,
							inputTextFlags,
							DebuggerCommandTextEditCallback,
							(void*)emulationGuiState.internalState))
		{
			SendCommandToDebugger(cmdBuffer,emulationGuiState);
		}

		if (ImGui::Button("Clock"))
		{
			SendCommandToDebugger("step -c 1",emulationGuiState);
		}
	}
}

static void DrawDebuggerGui(EmulationGuiState& emulationGuiState)
{
	ImGui::Begin("Register view");
		DrawDebuggerCPUStatus(emulationGuiState);
	ImGui::End();

	ImGui::Begin("Console");
		DrawDebuggerConsole(emulationGuiState);
	ImGui::End();


#if 0
	ImGui::BeginChild("Console",ImVec2(ImGui::GetContentRegionAvail().x*.5,0.0),true);
	DrawDebuggerCommandConsole(emulationGuiState.debuggerGuiState);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("Reg View", {0,0},true);
	ImGui::PushItemWidth(-FLT_MIN);
	ImGui::PopItemWidth();
	ImGui::EndChild();
#endif
}
#endif

extern "C" DLL_EXPORT HOT_RELOAD_DRAW_SIGNATURE
{
	if (emulationGuiState.internalState == nullptr)
	{
		emulationGuiState.internalState = new InternalState();
	}

	DrawEmulationConsole(emulationGuiState);
	DrawEmulationGui(emulationGuiState);

#ifdef DEBUGGER_ON
	DrawDebuggerGui(emulationGuiState);
#endif

	emulationGuiState.internalState->lastFrameCpuStat = emulationGuiState.emulationStat;
	emulationGuiState.internalState->lastFrameTime = HighResolutionClock::now();
}
