#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

namespace E5150::Debugger
{
	enum class SPECIAL_BEHAVIOUR
	{
		CALL, INTERRUPT, XCHG_BX_BX
	};

	void init (void);
	void wakeUp (const bool instructionExecuted);
	void specialBehaviour(const SPECIAL_BEHAVIOUR);
}

#endif