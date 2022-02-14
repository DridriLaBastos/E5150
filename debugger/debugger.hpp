#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

namespace E5150::Debugger
{
	enum class SPECIAL_BEHAVIOUR
	{
		CALL, INTERRUPT, XCHG_BX_BX
	};

	//TODO: tranform it to a class to have init as a constructor and deinint in destructor
	void init (void);
	void deinit (void);
	void wakeUp (const uint8_t instructionExecuted);
	void specialBehaviour(const SPECIAL_BEHAVIOUR);
}

#endif