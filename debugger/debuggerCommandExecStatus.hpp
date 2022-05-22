#ifndef __DEBUGGER_CMD_EXEC_STATUS__
#define __DEBUGGER_CMD_EXEC_STATUS__

#include "communication/command.h"

namespace E5150::DEBUGGER_COMMAND_EXECUTION
{
	struct CONTINUE_CONTEXT
	{
		static CONTINUE_TYPE TYPE;
		static unsigned int TARGET_VALUE;
		static unsigned int CURRENT_VALUE;
	};
}

#endif