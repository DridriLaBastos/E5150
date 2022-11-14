#include <mutex>
#include <thread>

#include "gui/gui.hpp"
#include "core/pch.hpp"
#include "core/arch.hpp"
#include "spdlog_imgui_color_sink.hpp"

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

void E5150::GUI::guiInit()
{
	spdlog::default_logger()->sinks().clear();
	spdlog::default_logger()->sinks().push_back(std::make_shared<SpdlogImGuiColorSink<std::mutex>>());
	auto imguiSink = (SpdlogImGuiColorSink<std::mutex>*)spdlog::default_logger()->sinks().back().get();
	imguiSink->init();
}

void E5150::GUI::drawGui()
{
	static auto consoleSink = (SpdlogImGuiColorSink<std::mutex>*)spdlog::default_logger()->sinks().back().get();
	spdlog::info("This is an info");
	spdlog::warn("This is a warning");
	spdlog::debug("This is a debug");
	spdlog::trace("This is a trace");

	consoleSink->imguiDraw();

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