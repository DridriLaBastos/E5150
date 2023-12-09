#ifndef __COMMANDS_HPP__
#define __COMMANDS_HPP__

#include "CLI11.hpp"

namespace E5150::DEBUGGER
{
	class AbstractCommand {
	public:
		AbstractCommand(const std::string& n, const std::string& d);
		virtual ~AbstractCommand(void);

		bool Parse(const std::string& line);

		virtual bool Step(const bool instructionExecuted, const bool instructionDecoded) = 0;

		const std::string  name, description;

	protected:
		virtual void InternalParse(CLI::App& app, std::string line) = 0;
	};

	namespace COMMANDS {
		class CommandContinue : public AbstractCommand {
		public:
			CommandContinue(void);
			virtual bool Step(const bool instructionExecuted, const bool instructionDecoded) override final;
		protected:
			virtual void InternalParse(CLI::App& app, std::string line) override final;
		};

		class CommandStep : public AbstractCommand {
		public:
			CommandStep(void);
			bool Step(const bool instructionExecuted, const bool instructionDecoded) final;
		private:
			virtual void InternalParse(CLI::App& app, std::string line) override final;
		};
	}
}

#endif