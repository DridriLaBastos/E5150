//
// Created by Adrien COURNAND on 24/12/2022.
//

#include "core/pch.hpp"
#include "core/emulation_constants.hpp"
#include "gui_states.hpp"
#include "third-party/imgui/imgui.h"
#include "spdlog_imgui_color_sink.hpp"
#include "gui_states.hpp"
#include "platform/platform.h"

char ImGuiTextBuffer::EmptyString[1] = { 0 };

using gui_clock = std::chrono::high_resolution_clock;

static constexpr unsigned int MS_PER_UPDATE = 1000;
static constexpr unsigned int EXPECTED_CPU_CLOCK_COUNT = E5150::CPU_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);
static constexpr unsigned int EXPECTED_FDC_CLOCK_COUNT = E5150::FDC_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);

extern "C" DLL_EXPORT void hotReloadDraw(const EmulationGUIState* const emulationGuiState)
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

	emulationGuiState->consoleSink->imguiDraw();

	const auto now = gui_clock::now();
	const auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime);
	if (timeSinceLastUpdate >= std::chrono::milliseconds(MS_PER_UPDATE))
	{
		instructionExecuted = emulationGuiState->instructionExecutedCount - lastFrameCPUInstructionCount;

		const uint64_t currentCPUClockCount = emulationGuiState->cpuClock;
		const uint64_t currentFDCClockCount = emulationGuiState->fdcClock;

		CPUClockDelta = currentCPUClockCount - lastFrameCPUClockCount;
		FDCClockDelta = currentFDCClockCount - lastFrameFDCClockCount;

		CPUClockAccuracy = CPUClockDelta * 100 / EXPECTED_CPU_CLOCK_COUNT;
		FDCClockAccuracy = FDCClockDelta * 100 / EXPECTED_FDC_CLOCK_COUNT;

		lastFrameCPUInstructionCount = emulationGuiState->instructionExecutedCount;
		lastFrameCPUClockCount = currentCPUClockCount;
		lastFrameFDCClockCount = currentFDCClockCount;

		lastUpdateTime = now;
	}

	ImGui::Begin("Emulation Statistics");

	ImGui::Text("CPU clock executed : %6d / %6d", CPUClockDelta, EXPECTED_CPU_CLOCK_COUNT);
	ImGui::Text("FDC clock executed : %6d / %6d", FDCClockDelta, EXPECTED_FDC_CLOCK_COUNT);
	ImGui::Text("Clock accuracy cpu : %d%%  fdc %d%%", CPUClockAccuracy, FDCClockAccuracy);
	ImGui::Text("Instruction executed : %.3fM/s ( %4.3fM)",instructionExecuted / 1e6, emulationGuiState->instructionExecutedCount / 1e6);
	// ImGui::TextUnformatted("Bonjour");

	ImGui::End();
}
