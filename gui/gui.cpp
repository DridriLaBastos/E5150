// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "gui/gui.hpp"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void E5150::GUI::UILoop(void)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    	
        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

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
#define DRAW_LIBRARY_COPY_FILE_NAME DRAW_LIBRARY_PREFIX_BASE_NAME "_c" DRAW_LIBRARY_FILE_EXTENSION

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
		E5150_WARNING("Unable to access draw library modification time : {}", platformGetLastErrorDescription());
		goto errorDrawFunctionUnchanged;
	}

	if (lastWriteTime == libDrawLastWriteTime)
	{ return; }

	libDrawLastWriteTime = lastWriteTime;

	//TODO: Why this fails on Windows ?
	#ifndef _WIN32
	fs::copy_file(DRAW_LIBRARY_FULL_PATH,DRAW_LIBRARY_COPY_FILE_NAME,fs::copy_options::overwrite_existing,errorCode);
	#else
	const PLATFORM_CODE code = platformFile_Copy(DRAW_LIBRARY_FULL_PATH,DRAW_LIBRARY_COPY_FILE_NAME);
	errorCode.clear();

	if (code)
	{
		errorCode.assign(platformGetLastErrorCode(), std::system_category());
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
		E5150_WARNING("Unable to unload the draw library : {}", platformGetLastErrorDescription());
		goto errorDrawFunctionUnchanged;
	}

	hotReloadModuleID = platformDylib_Load(DRAW_LIBRARY_COPY_FILE_NAME);

	if (hotReloadModuleID < 0)
	{
		E5150_WARNING("Unable to reload the draw library file : {}", errorCode.message());
		goto errorDrawFunctionReset;
	}

	if (platformDylib_GetSymbolAddress(hotReloadModuleID,HOT_RELOAD_DRAW_NAME,(void**)&hotReloadDraw))
	{
		E5150_ERROR("Unable to update draw function : {}",errorCode.message());
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
#endif

#ifdef DEBUGGER_ON
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
	//t = std::thread(&E5150::Arch::startSimulation,&arch);
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

		hotReloadDraw(emulationGuiData, debuggerGuiData);
	}
}

void E5150::GUI::clean()
{
	E5150::Util::_continue = false;
#ifdef DEBUGGER_ON
	E5150::DEBUGGER::clean();
#endif
	//t.join();
}
