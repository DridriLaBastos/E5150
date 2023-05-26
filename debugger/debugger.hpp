#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include "argparse/argparse.hpp"

namespace E5150::DEBUGGER
{
	class Command {
	public:
		Command(const std::string& n, const std::string& d);
		virtual ~Command(void);

		void Prepare(const std::vector<std::string>& args);

		virtual bool Step(const bool instructionExecuted, const bool instructionDecoded) = 0;

		const std::string  name, description;

	protected:
		virtual void InternalPrepare(void);

	protected:
		argparse::ArgumentParser mParser;
	};

	void init (void);
	void clean (void);
	void wakeUp (const uint8_t instructionExecuted, const bool instructionDecoded);
	bool Launch(const std::string &commandName, const std::vector<std::string> argv);
}

#endif
