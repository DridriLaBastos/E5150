#include "debugger.hpp"
#include "core/util.hpp"
#include "core/arch.hpp"
#include "third-party/imgui/imgui.h"
#include "communication/communication.h"

using namespace E5150::Debugger::GUI;

static std::thread pullDebuggerStdoutThread;
static std::thread pullDebuggerStderrThread;
static std::mutex imguiDebugConsoleMutex;

static std::vector<E5150::Debugger::GUI::ConsoleEntry> debugConsoleEntries;

//TODO: Adding multiple time a std::string to the data means storing
//additional '\0' end character. With a lot of logs this could be
//a waste of memory.
/*static int execCommand(void)
{
	//TODO: asynchronous send command to backend and wait for an answer
	debugConsoleEntries.emplace_back(CONSOLE_ENTRY_TYPE::COMMAND,debuggerCommandInputBuffer);
	const size_t commandLength = strnlen(&debuggerCommandInputBuffer[3],256-3);
	WRITE_TO_DEBUGGER(&commandLength,sizeof(commandLength));
	WRITE_TO_DEBUGGER(&debuggerCommandInputBuffer[3],commandLength);
	return 0;
}*/

static void pullChildStreamThreadFunction(PLATFORM_CODE(*childStreamPullFunction)(char* const),const CONSOLE_ENTRY_TYPE type)
{
	std::string line;
	char c;
	bool streamOpen = true;
	PLATFORM_CODE streamPullCode;

	while (streamOpen)
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
				E5150_DEBUG("Received EOF from debugger {} stream", type == CONSOLE_ENTRY_TYPE::DEBUGGER_STDOUT ? "stdout" : "stderr");
			}
			//TODO: quit at each error. Maybe we want to have a case by case treatment. We could try to read
			//again from the stream depending on he error and if its not EOM
			streamOpen = false;
		}
		else
		{
			imguiDebugConsoleMutex.lock();
			debugConsoleEntries.emplace_back(type,line);
			printf("%s",line.c_str());
			imguiDebugConsoleMutex.unlock();
			line.clear();
		}
	}
}

static void pullStdoutThreadFunction() {
	pullChildStreamThreadFunction(platformReadChildSTDOUT, CONSOLE_ENTRY_TYPE::DEBUGGER_STDOUT); }
static void pullStderrThreadFunction() {
	pullChildStreamThreadFunction(platformReadChildSTDERR, CONSOLE_ENTRY_TYPE::DEBUGGER_STDERR); }

void E5150::Debugger::sendCommand()
{
	static size_t lastCommandEntryIndex = 0;

	for (size_t i = lastCommandEntryIndex+1; i < debugConsoleEntries.size(); i += 1)
	{
		if (debugConsoleEntries[i].type == CONSOLE_ENTRY_TYPE::COMMAND)
		{
			lastCommandEntryIndex = i;
			const auto& entry = debugConsoleEntries[i];
			const size_t commandLength = entry.str.size();
			WRITE_TO_DEBUGGER(&commandLength,sizeof(commandLength));
			WRITE_TO_DEBUGGER(entry.str.c_str(),commandLength);
			break;
		}
	}
}

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

/*void E5150::Debugger::GUI::draw()
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
			case CONSOLE_ENTRY_TYPE::COMMAND:
				ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.0f, 0.8f, 0.6f, 1.0f));
				break;
			case CONSOLE_ENTRY_TYPE::DEBUGGER_STDERR:
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
		//execCommand();
	}

	ImGui::End();
}*/

const E5150::Debugger::GUI::State E5150::Debugger::GUI::getState()
{
	E5150::Debugger::GUI::State state;
	state.debugConsoleEntries = &debugConsoleEntries;
	state.debugConsoleMutex = &imguiDebugConsoleMutex;
	state.i8086 = &E5150::Arch::_cpu;
	state.debuggerIsRunning = E5150::Debugger::getDebuggerIsRunningState();

	return state;
}
