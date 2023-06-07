#ifndef __COMMANDS_HPP__
#define __COMMANDS_HPP__

#include "CLI11.hpp"

namespace E5150::DEBUGGER
{
	class AbstractCommand {
	public:
		AbstractCommand(const std::string& n, const std::string& d);
		virtual ~AbstractCommand(void);

		void Prepare(const std::vector<std::string>& args);

		virtual bool Step(const bool instructionExecuted, const bool instructionDecoded) = 0;

		const std::string  name, description;

	protected:
		virtual void MakeReady(void);
		virtual void OnCommandParsed(void);

	protected:
		CLI::App mApp;
	};

	namespace COMMANDS {
		class CommandContinue : public AbstractCommand {
		public:
			CommandContinue(void);

			bool Step(const bool instructionExecuted, const bool instructionDecoded) final;
			virtual void OnCommandParsed(void) override final;
			virtual void MakeReady(void) override final;
		};
	}
}

#endif