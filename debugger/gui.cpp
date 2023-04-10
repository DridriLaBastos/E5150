#include "debugger.hpp"
#include "core/util.hpp"
#include "core/arch.hpp"
#include "third-party/imgui/imgui.h"
#include "communication.h"

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
			WRITE_TO_DEBUGGER((void*)&commandLength,sizeof(commandLength));
			WRITE_TO_DEBUGGER((void*)entry.str.c_str(),commandLength);
			break;
		}
	}
}

static void pullChildStreamThreadFunction(const CONSOLE_ENTRY_TYPE type)
{
	std::string line;
	char c;
	bool streamOpen = true;
	const size_t freadNitemToRead = 1;
	const size_t freadBytesToRead = freadNitemToRead * sizeof(c);
	size_t freadReturnValue;
	//TODO: intended error while developing debugger <-> communication
	FILE* stream = E5150::Debugger::getDebuggerStdStream(type == CONSOLE_ENTRY_TYPE::DEBUGGER_STDOUT ?
			E5150::Debugger::DEBUGGER_STD_STREAM::STDOUT : E5150::Debugger::DEBUGGER_STD_STREAM::STDERR);

	while (streamOpen)
	{
		do {
			freadReturnValue = fread(&c,freadBytesToRead,freadNitemToRead,stream);
			line.push_back(c);
		} while ((freadReturnValue == freadBytesToRead) && (c != '\n'));

		if (freadReturnValue != freadBytesToRead)
		{
			const int streamEof = feof(stream);
			const int streamError = ferror(stream);

			if (streamEof)
			{
				E5150_DEBUG("Received stream EOF");
			}

			if (streamError)
			{
				E5150_WARNING("Error while reading debuger stream - ERRNO({}) : '{}'",errno, strerror(errno));
			}

			streamOpen = false;
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

void E5150::Debugger::GUI::init()
{
	pullDebuggerStdoutThread = std::thread(pullChildStreamThreadFunction,CONSOLE_ENTRY_TYPE::DEBUGGER_STDOUT);
	pullDebuggerStderrThread = std::thread(pullChildStreamThreadFunction,CONSOLE_ENTRY_TYPE::DEBUGGER_STDERR);
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
