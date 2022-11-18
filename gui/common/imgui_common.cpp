#include "gui/gui.hpp"
#include "core/pch.hpp"
#include "core/arch.hpp"
#include "spdlog_imgui_color_sink.hpp"

#ifdef DEBUGGER
#include "debugger/debugger.hpp"
#endif

using gui_clock = std::chrono::high_resolution_clock;

static constexpr unsigned int MS_PER_UPDATE = 1000;
static constexpr unsigned int EXPECTED_CPU_CLOCK_COUNT = E5150::Arch::CPU_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);
static constexpr unsigned int EXPECTED_FDC_CLOCK_COUNT = E5150::Arch::FDC_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);

static void simulationThread() {
#ifdef DEBUGGER
	E5150_INFO("Emulation with debugger (loglevel: {})",E5150::Util::CURRENT_EMULATION_LOG_LEVEL);
	spdlog::set_level(spdlog::level::debug);
#endif
	E5150::Arch arch;

#if 1
	//Loading IBM BIOS
	ram.load("test/ibm_bios.bin", 0xFE000);
#else
	//Loading custom test code
		ram.load("test/interrupts.bin",0);
		ram.load("test/jmp.bin", 0xFFFF0);
		//ram.load("/Users/adrien/Documents/Informatique/OS/Beetle16/init/init.bin",0x500);
		ram.load("test/bios.bin",0x500);
#endif

	arch.startSimulation();
}

static std::thread t;

static void stop(const int signum)
{
	puts("Please close the window to quit");
}

void E5150::GUI::guiInit()
{
	spdlog::default_logger()->sinks().clear();
	spdlog::default_logger()->sinks().push_back(std::make_shared<SpdlogImGuiColorSink<std::mutex>>());
	auto imguiSink = (SpdlogImGuiColorSink<std::mutex>*)spdlog::default_logger()->sinks().back().get();
	imguiSink->init();

	#ifndef WIN32 //Those signals values aren't defined in windows
		signal(SIGSTOP, stop);
		signal(SIGQUIT, stop);
		signal(SIGKILL, stop);
	#endif
	
	signal(SIGABRT, stop);
	signal(SIGINT, stop);
	signal(SIGTERM, stop);

	#ifdef DEBUGGER
		Debugger::init();
	#endif

	t = std::thread(simulationThread);
}

void E5150::GUI::guiDraw()
{
	static auto consoleSink = (SpdlogImGuiColorSink<std::mutex>*)spdlog::default_logger()->sinks().back().get();
	static unsigned int instructionExecuted = 0;
	static uint64_t lastFrameCPUClockCount = 0;
	static uint64_t lastFrameFDCClockCount = 0;
	static unsigned int CPUClockDelta = 0;
	static unsigned int FDCClockDelta = 0;
	static unsigned int CPUClockAccuracy = 0;
	static unsigned int FDCClockAccuracy = 0;
	static auto lastUpdateTime = gui_clock::now();

	consoleSink->imguiDraw();

	const auto now = gui_clock::now();
	const auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime);
	if (timeSinceLastUpdate >= std::chrono::milliseconds(MS_PER_UPDATE))
	{
		const unsigned int deltaInstructions = cpu.instructionExecutedCount - instructionExecuted;

		const uint64_t currentCPUClockCount = Arch::emulationStat.cpuClock;
		const uint64_t currentFDCClockCount = Arch::emulationStat.fdcClock;

		CPUClockDelta = currentCPUClockCount - lastFrameCPUClockCount;
		FDCClockDelta = currentFDCClockCount - lastFrameFDCClockCount;

		CPUClockAccuracy = CPUClockDelta * 100 / EXPECTED_CPU_CLOCK_COUNT;
		FDCClockAccuracy = FDCClockDelta * 100 / EXPECTED_FDC_CLOCK_COUNT;

		instructionExecuted = Arch::emulationStat.instructionExecutedCount;
		lastFrameCPUClockCount = currentCPUClockCount;
		lastFrameFDCClockCount = currentFDCClockCount;

		lastUpdateTime = now;
	}

	ImGui::Begin("Emulation Statistics");
	ImGui::Text("CPU clock executed : %6d / %6d", CPUClockDelta, EXPECTED_CPU_CLOCK_COUNT);
	ImGui::Text("FDC clock executed : %6d / %6d", FDCClockDelta, EXPECTED_FDC_CLOCK_COUNT);
	ImGui::Text("Clock accuracy cpu : %d%%  fdc %d%%", CPUClockAccuracy, FDCClockAccuracy);
	ImGui::Text("Instruction executed : %.3fM", (float)Arch::emulationStat.instructionExecutedCount/1e6);
	ImGui::End();

	/*if (!simulationStarted)
	{
		ImGui::Begin("Home");
		const bool startSimulationRequest = ImGui::Button("Start simulation");
		ImGui::End();

		if (startSimulationRequest && !simulationStarted) {
			E5150_INFO("Simulation is starting !");
			const auto simThread = std::thread(simulationThread);
			simulationStarted = true;
		}
	}*/
}

void E5150::GUI::guiDeinit()
{
	E5150::Util::_continue = false;
	t.join();
#ifdef DEBUGGER
	Debugger::deinit();
#endif
}
