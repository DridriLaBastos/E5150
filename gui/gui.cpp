#include <csignal>
#include <filesystem>

#include "gui/gui.hpp"
#include "core/pch.hpp"
#include "core/arch.hpp"
#include "gui_states.hpp"
#include "debugger/cli.hpp"
#include "platform/platform.h"
#include "debugger/debugger.hpp"
#include "spdlog_imgui_color_sink.hpp"
#include "core/emulation_constants.hpp"

//char* HOT_RELOAD_DRAW_NAME = "hotReloadDraw";
#define HOT_RELOAD_DRAW_NAME "hotReloadDraw"

using gui_clock = std::chrono::high_resolution_clock;

namespace fs = std::filesystem;

static constexpr unsigned int MS_PER_UPDATE = 1000;
static constexpr unsigned int EXPECTED_CPU_CLOCK_COUNT = E5150::CPU_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);
static constexpr unsigned int EXPECTED_FDC_CLOCK_COUNT = E5150::FDC_BASE_CLOCK * (MS_PER_UPDATE / 1000.f);

static void (*hotReloadDraw)(const EmulationGUIState&, const DebuggerGuiData&) = nullptr;
static module_t hotReloadModuleID = -1;
fs::file_time_type libDrawLastWriteTime;
static std::error_code errorCode;

static std::thread t;

static void stop(const int signum)
{
	puts("Please close the window to quit");
}

static void reloadDrawLibrary()
{
	const fs::file_time_type lastWriteTime = fs::last_write_time(DRAW_LIBRARY_FULL_PATH, errorCode);

	if (errorCode)
	{
		E5150_WARNING("Unable to access draw library modification time : {}", platformError_GetCode());
		goto errorDrawFunctionUnchanged;
	}

	if (lastWriteTime == libDrawLastWriteTime)
	{ return; }

	libDrawLastWriteTime = lastWriteTime;

	//TODO: Why this fails on Windows ?
	#ifndef _WIN32
	fs::copy_file(DRAW_LIBRARY_FULL_PATH,DRAW_LIBRARY_COPY_FULL_PATH,fs::copy_options::overwrite_existing,errorCode);
	#else
	const PLATFORM_CODE code = platformFile_Copy(DRAW_LIBRARY_FULL_PATH,DRAW_LIBRARY_COPY_FULL_PATH);
	errorCode.clear();

	if (code)
	{
		errorCode.assign(platformError_GetCode(), std::system_category());
	}
	#endif
	if (errorCode)
	{
		E5150_WARNING("Unable to make a copy of the draw library : {}", errorCode.message());
		goto errorDrawFunctionUnchanged;
	}

	//TODO: Is this an error ? Why would it possible for Windows to fail to unload a dylib only used here ?
	if (platformDylib_Release(hotReloadModuleID))
	{
		E5150_WARNING("Unable to unload the draw library : {}", platformError_GetDescription());
		goto errorDrawFunctionUnchanged;
	}

	hotReloadModuleID = platformDylib_Load(DRAW_LIBRARY_COPY_FULL_PATH);

	if (hotReloadModuleID < 0)
	{
		E5150_WARNING("Unable to reload the draw library file : ERROR({}) : {}", platformError_GetCode(), platformError_GetDescription());
		goto errorDrawFunctionReset;
	}

	if (platformDylib_GetSymbolAddress(hotReloadModuleID,HOT_RELOAD_DRAW_NAME,(void**)&hotReloadDraw))
	{
		E5150_ERROR("Unable to reload the draw library file : ERROR({}) : {}", platformError_GetCode(), platformError_GetDescription());
		goto errorDrawFunctionReset;
	}

	E5150_INFO("Draw function successfully reloaded");
	return;

	errorDrawFunctionUnchanged:
	E5150_WARNING("\tDraw function not updated, previous version used");
	return;

	errorDrawFunctionReset:
	E5150_WARNING("\tGUI not draw");
	hotReloadModuleID = -1;
	hotReloadDraw = nullptr;
	//TODO: remake spdlog output to the console
	return;
}

void E5150::GUI::init()
{
	reloadDrawLibrary();

	if (hotReloadDraw)
	{
		//spdlog::default_logger()->sinks().clear();
		spdlog::default_logger()->sinks().push_back(std::make_shared<SpdlogImGuiColorSink<std::mutex>>());
		auto imguiSink = (SpdlogImGuiColorSink<std::mutex>*)spdlog::default_logger()->sinks().back().get();
		imguiSink->init();
	}

	E5150_INFO("Welcome to E5150, the emulator of an IBM PC 5150");

#ifdef DEBUGGER_ON
	E5150_INFO("Emulation with debugger (loglevel: {})",E5150::Util::CURRENT_EMULATION_LOG_LEVEL);
	spdlog::set_level(spdlog::level::debug);
	E5150::DEBUGGER::init();
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

#ifdef DEBUGGER_ON
	E5150::DEBUGGER::PrepareGuiSide();
#endif
}

void E5150::GUI::draw()
{
	reloadDrawLibrary();

	if (hotReloadDraw)
	{
		EmulationGUIState emulationGuiData;
		emulationGuiData.cpuClock = E5150::Arch::emulationStat.cpuClock;
		emulationGuiData.fdcClock = E5150::Arch::emulationStat.fdcClock;
		emulationGuiData.instructionExecutedCount = E5150::Arch::emulationStat.instructionExecutedCount;
		emulationGuiData.consoleSink = (SpdlogImGuiColorSink<std::mutex>*)spdlog::default_logger()->sinks().back().get();

		DebuggerGuiData debuggerGuiData;
		debuggerGuiData.parseLine = E5150::DEBUGGER::CLI::ParseLine;
		debuggerGuiData.i8086 = &E5150::Arch::_cpu;

		hotReloadDraw(emulationGuiData, debuggerGuiData);
	}
}

void E5150::GUI::clean()
{
	E5150::Util::_continue = false;
#ifdef DEBUGGER_ON
	E5150::DEBUGGER::clean();
#endif
	t.join();
}
