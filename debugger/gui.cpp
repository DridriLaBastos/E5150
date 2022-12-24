#include "debugger.hpp"
#include "core/util.hpp"
#include "third-party/imgui/imgui.h"
#include "communication/communication.h"

static char debuggerCommandInputBuffer [256] = { ' ', '#', ' '};

static std::thread pullDebuggerStdoutThread;
static std::thread pullDebuggerStderrThread;
static std::mutex imguiDebugConsoleMutex;

enum class DEBUG_CONSOLE_ENTRY_TYPE
{ COMMAND, DEBUGGER_STDERR, DEBUGGER_STDOUT };

static std::vector<std::pair<DEBUG_CONSOLE_ENTRY_TYPE,std::string>> debugConsoleEntries;

//TODO: Adding multiple time a std::string to the data means storing
//additional '\0' end character. With a lot of logs this could be
//a waste of memory.
static int execCommand(void)
{
	//TODO: asynchronous send command to backend and wait for an answer
	debugConsoleEntries.emplace_back(DEBUG_CONSOLE_ENTRY_TYPE::COMMAND,debuggerCommandInputBuffer);
	const size_t commandLength = strnlen(&debuggerCommandInputBuffer[3],256-3);
	WRITE_TO_DEBUGGER(&commandLength,sizeof(commandLength));
	WRITE_TO_DEBUGGER(&debuggerCommandInputBuffer[3],commandLength);
	return 0;
}

static void pullChildStreamThreadFunction(PLATFORM_CODE(*childStreamPullFunction)(char* const),const DEBUG_CONSOLE_ENTRY_TYPE type)
{
	std::string line;
	char c;
	bool streamOpened = true;
	PLATFORM_CODE streamPullCode;

	while (streamOpened)
	{
		do {
			streamPullCode = childStreamPullFunction(&c);
			line.push_back(c);
		} while ((streamPullCode == PLATFORM_SUCCESS) && (c != '\n'));

		if (streamPullCode != PLATFORM_SUCCESS)
		{
			if (streamPullCode == PLATFORM_ERROR)
			{
				E5150_DEBUG("Exit with error {}", platformGetLastErrorCode());
				E5150_ERROR(platformGetLastErrorDescription());
			}
			else
			{
				E5150_DEBUG("Received EOF from debugger {} stream", type == DEBUG_CONSOLE_ENTRY_TYPE::DEBUGGER_STDOUT ? "stdout" : "stderr");
			}
			//TODO: quit at each error. Maybe we want to have a case by case treatment. We could try to read
			//again from the stream depending on he error and if its not EOM
			streamOpened = false;
		}
		else
		{
			imguiDebugConsoleMutex.lock();
			debugConsoleEntries.emplace_back(type,line);
			imguiDebugConsoleMutex.unlock();
			line.clear();
		}
	}
}

static void pullStdoutThreadFunction() {
	pullChildStreamThreadFunction(platformReadChildSTDOUT, DEBUG_CONSOLE_ENTRY_TYPE::DEBUGGER_STDOUT); }
static void pullStderrThreadFunction() {
	pullChildStreamThreadFunction(platformReadChildSTDERR, DEBUG_CONSOLE_ENTRY_TYPE::DEBUGGER_STDERR); }

void E5150::Debugger::GUI::init()
{
	pullDebuggerStdoutThread = std::thread(pullStdoutThreadFunction);
	pullDebuggerStderrThread = std::thread(pullStderrThreadFunction);
}

void E5150::Debugger::GUI::clean()
{
	pullDebuggerStdoutThread.join();
	pullDebuggerStderrThread.join();
}

void E5150::Debugger::GUI::draw()
{
	ImGui::Begin("Debugger");

	imguiDebugConsoleMutex.lock();
	size_t s = debugConsoleEntries.size();
	imguiDebugConsoleMutex.unlock();

	for (size_t i = 0; i < s; ++i)
	{
		bool hasColor = true;
		//Since a push back can potentialy invalidate data inside a vector, we need to lock it during the whole
		//iteration
		imguiDebugConsoleMutex.lock();
		const auto& entry = debugConsoleEntries[i];
		switch(entry.first)
		{
			case DEBUG_CONSOLE_ENTRY_TYPE::COMMAND:
				ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.0f, 0.8f, 0.6f, 1.0f));
				break;
			case DEBUG_CONSOLE_ENTRY_TYPE::DEBUGGER_STDERR:
				ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.0f, 0.1f, 0.2f, 1.0f));
				break;
			default:
				hasColor = false;
				break;
		}
		ImGui::TextUnformatted(entry.second.c_str());
		if (hasColor) { ImGui::PopStyleColor(); }
		s = debugConsoleEntries.size();
		imguiDebugConsoleMutex.unlock();
	}

	if (ImGui::InputText("Debugger command",&debuggerCommandInputBuffer[3],IM_ARRAYSIZE(debuggerCommandInputBuffer)-3,ImGuiInputTextFlags_EnterReturnsTrue))
	{
		execCommand();
	}

	ImGui::End();
}