#ifndef __DEBUGGER_GUI_HPP__
#define __DEBUGGER_GUI_HPP__

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "argparse/argparse.hpp"

namespace E5150::DEBUGGER
{
	enum COMMAND_LAUNCH_RETURN {
		COMMAND_EXIT_FAILURE,
		COMMAND_EXIT_SUCCESS_NO_RUNNING,
		COMMAND_EXIT_SUCCESS_RUNNING
	};

	class Command {
		public:
			Command(const std::string& n, const std::string d);
			virtual ~Command(void);

			void Launch(const std::vector<std::string>& args);
			bool Step(const bool instructionExecuted);

			bool GetExecutionStatus(void);

			const std::string name;
			const std::string description;

	protected:
		virtual COMMAND_LAUNCH_RETURN InternalLaunch(const std::vector<std::string>& args) = 0;
		virtual bool InternalStep(const bool instructionExecuted) = 0;
		
		protected:
			argparse::ArgumentParser mParser;
			bool mCommandIsRunning;
	};

	class CLI {
		public:
			static void ParseLine(const std::string& cmdLine);
			static void CommandFinished(
#ifdef _DEBUG
										Command* cmd //On Debug compilation, check that command requesting a command
													 //is the one that is running
#else
										void
#endif
										);
			static Command* GetRunningCommand(void);

			template<class _Cmd, class ...Args>
			static Command& RegisterCommand(Args... args)
			{
				commands.emplace_back(std::make_unique<_Cmd>(std::forward(args)...));
				return *(commands.back().get());
			}

		private:
			static std::vector<std::unique_ptr<Command>> commands;
			static Command* runningCommand;
	};
}

#endif