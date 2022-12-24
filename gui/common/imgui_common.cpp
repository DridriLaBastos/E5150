#include <csignal>

#include "gui/gui.hpp"
#include "core/pch.hpp"
#include "core/arch.hpp"
#include "spdlog_imgui_color_sink.hpp"
#include "debugger/platform.h"

#ifdef DEBUGGER
#include "debugger/debugger.hpp"
#endif

//char* HOT_RELOAD_DRAW_NAME = "hotReloadDraw";
#define HOT_RELOAD_DRAW_NAME "hotReloadDraw"

using gui_clock = std::chrono::high_resolution_clock;

static constexpr unsigned int MS_PER_UPDATE = 1000;
static constexpr unsigned int EXPECTED_CPU_CLOCK_COUNT = E5150::Arch::CPU_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);
static constexpr unsigned int EXPECTED_FDC_CLOCK_COUNT = E5150::Arch::FDC_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);

static void(*hotReloadDraw)(void) = nullptr;
static uint64_t hotReloadModuleDrawLastModificationTime = 0;
static module_t hotReloadModuleID = 0;

static std::thread t;

static void stop(const int signum)
{
	puts("Please close the window to quit");
}

void E5150::GUI::init()
{
	hotReloadModuleID = platformDylib_Load(HOT_RELOAD_DRAW_PATH);

	if (hotReloadModuleID < 0)
	{
		E5150_WARNING("Unable to load UI drawing code : {}",platformGetLastErrorDescription());
	}
	else
	{
		if (platformFile_GetLastModificationTime(HOT_RELOAD_DRAW_PATH,&hotReloadModuleDrawLastModificationTime))
		{
			E5150_WARNING("Unable to get UI draw library access time : {}",platformGetLastErrorDescription());
		}

		if (platformDylib_GetSymbolAddress(hotReloadModuleID,HOT_RELOAD_DRAW_NAME,(void**)&hotReloadDraw))
		{
			E5150_WARNING("Unable to get UI draw function : {}", platformGetLastErrorDescription());
		}
	}

	if (hotReloadDraw)
	{
		spdlog::default_logger()->sinks().clear();
		spdlog::default_logger()->sinks().push_back(std::make_shared<SpdlogImGuiColorSink<std::mutex>>());
		auto imguiSink = (SpdlogImGuiColorSink<std::mutex>*)spdlog::default_logger()->sinks().back().get();
		imguiSink->init();
	}

	E5150_INFO("Welcome to E5150, the emulator of an IBM PC 5150");
#ifdef DEBUGGER
	E5150_INFO("Emulation with debugger (loglevel: {})",E5150::Util::CURRENT_EMULATION_LOG_LEVEL);
	spdlog::set_level(spdlog::level::debug);
#endif

#ifdef DEBUGGER
	E5150::Debugger::init();
	E5150::Debugger::GUI::init();
#endif

	E5150::Arch arch;

	#ifndef WIN32 //Those signals values aren't defined in windows
		signal(SIGSTOP, stop);
		signal(SIGQUIT, stop);
		signal(SIGKILL, stop);
	#endif

	signal(SIGABRT, stop);
	signal(SIGINT, stop);
	signal(SIGTERM, stop);

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

	//TODO: launching the thread arch shouldn't be done inside the gui init function
	t = std::thread(&E5150::Arch::startSimulation,&arch);
}

static void reloadHotReloadDrawFunction(void)
{
	void* lastLoadedCode = hotReloadDraw;
	void* newCode = nullptr;

	if (platformDylib_GetSymbolAddress(hotReloadModuleID,HOT_RELOAD_DRAW_NAME,&newCode))
	{
		E5150_WARNING("Unable to reload UI draw function : {}",platformGetLastErrorDescription());
		E5150_WARNING("Previously loaded code will be used");
		newCode = lastLoadedCode;
	}

	hotReloadDraw = (void(*)(void))newCode;
}

void E5150::GUI::draw()
{
	uint64_t accessTime;
	const PLATFORM_CODE platformCode = platformFile_GetLastModificationTime(HOT_RELOAD_DRAW_PATH,&accessTime);

	if (accessTime != hotReloadModuleDrawLastModificationTime)
	{
		hotReloadModuleDrawLastModificationTime = accessTime;

		if (platformCode == PLATFORM_ERROR)
		{
			E5150_WARNING("Cannot open drawing function code : {}",platformGetLastErrorDescription());
			E5150_WARNING("Previous code will be used");
		}
		else
		{
			//TODO: Reload drawing function code
		}
	}

	if (hotReloadDraw)
	{
		hotReloadDraw();
	}
#if 0
	static auto consoleSink = (SpdlogImGuiColorSink<std::mutex>*)spdlog::default_logger()->sinks().back().get();
	static unsigned int instructionExecuted = 0;
	static uint64_t lastFrameCPUClockCount = 0;
	static unsigned int lastFrameCPUInstructionCount = 0;
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
		instructionExecuted = cpu.instructionExecutedCount - lastFrameCPUInstructionCount;

		const uint64_t currentCPUClockCount = Arch::emulationStat.cpuClock;
		const uint64_t currentFDCClockCount = Arch::emulationStat.fdcClock;

		CPUClockDelta = currentCPUClockCount - lastFrameCPUClockCount;
		FDCClockDelta = currentFDCClockCount - lastFrameFDCClockCount;

		CPUClockAccuracy = CPUClockDelta * 100 / EXPECTED_CPU_CLOCK_COUNT;
		FDCClockAccuracy = FDCClockDelta * 100 / EXPECTED_FDC_CLOCK_COUNT;

		lastFrameCPUInstructionCount = cpu.instructionExecutedCount;
		lastFrameCPUClockCount = currentCPUClockCount;
		lastFrameFDCClockCount = currentFDCClockCount;

		lastUpdateTime = now;
	}

	ImGui::Begin("Emulation Statistics");
	ImGui::Text("CPU clock executed : %6d / %6d", CPUClockDelta, EXPECTED_CPU_CLOCK_COUNT);
	ImGui::Text("FDC clock executed : %6d / %6d", FDCClockDelta, EXPECTED_FDC_CLOCK_COUNT);
	ImGui::Text("Clock accuracy cpu : %d%%  fdc %d%%", CPUClockAccuracy, FDCClockAccuracy);
	ImGui::Text("Instruction executed : %.3fM",instructionExecuted / 1e6);
	ImGui::End();

#ifdef DEBUGGER
	E5150::Debugger::GUI::draw();
#endif

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
#endif
}

void E5150::GUI::clean()
{
	E5150::Util::_continue = false;
#ifdef DEBUGGER
	E5150::Debugger::clean();
#endif
	t.join();
}
