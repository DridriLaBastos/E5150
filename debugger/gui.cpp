#include "debugger.hpp"
#include "third-party/imgui/imgui.h"

static char debuggerCommandInputBuffer [256] = { ' ', '#', ' '};

enum class DEBUG_CONSOLE_ENTRY_TYPE
{ COMMAND, RESPONSE };

static std::vector<std::pair<DEBUG_CONSOLE_ENTRY_TYPE,std::string>> consoleEntries;

//TODO: Adding multiple time a std::string to the data means storing
//additional '\0' end character. With a lot of logs this could be
//a waste of memory.
static int execCommand(void)
{
	//TODO: asynchronous send command to backend and wait for an answer
	consoleEntries.emplace_back(DEBUG_CONSOLE_ENTRY_TYPE::COMMAND,debuggerCommandInputBuffer);
	consoleEntries.emplace_back(DEBUG_CONSOLE_ENTRY_TYPE::RESPONSE, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
}

void E5150::Debugger::GUI::drawConsole()
{
	ImGui::Begin("Debugger");

	//TODO: Use ImGui clipper (see comment on the ExampleConsole of ImGui)
	std::for_each(consoleEntries.begin(), consoleEntries.end(), [](const std::pair<DEBUG_CONSOLE_ENTRY_TYPE,std::string> entry){
		bool hasColor = false;
		if (entry.first == DEBUG_CONSOLE_ENTRY_TYPE::COMMAND)
		{ ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.0f, 0.8f, 0.6f, 1.0f)); hasColor=true; }
		ImGui::TextUnformatted(entry.second.c_str());
		if (hasColor) { ImGui::PopStyleColor(); }
	});

	if (ImGui::InputText("Debugger command",&debuggerCommandInputBuffer[3],IM_ARRAYSIZE(debuggerCommandInputBuffer)-3,ImGuiInputTextFlags_EnterReturnsTrue))
	{
		execCommand();
	}

	ImGui::End();
}