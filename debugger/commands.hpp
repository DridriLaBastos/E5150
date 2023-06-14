#ifndef __COMMANDS_HPP__
#define __COMMANDS_HPP__

#include "CLI11.hpp"

namespace E5150::DEBUGGER
{
	class AbstractCommand {
	public:
		AbstractCommand(const std::string& n, const std::string& d);
		virtual ~AbstractCommand(void);

		bool Parse(const std::vector<std::string>& argv);

		virtual bool Step(const bool instructionExecuted, const bool instructionDecoded) = 0;

		const std::string  name, description;

	protected:
		virtual void InternalParse(CLI::App& app, std::vector<std::string>& argv) = 0;
	};

	namespace COMMANDS {
		class CommandContinue : public AbstractCommand {
		public:
			CommandContinue(void);
			virtual bool Step(const bool instructionExecuted, const bool instructionDecoded) override final;
		protected:
			virtual void InternalParse(CLI::App& app, std::vector<std::string>& argv) override final;
		};

		class CommandStep : public AbstractCommand {
		public:
			CommandStep(void);
			bool Step(const bool instructionExecuted, const bool instructionDecoded) final;
		private:
			virtual void InternalParse(CLI::App& app, std::vector<std::string>& argv) override final;
		};
	}
}

#endif