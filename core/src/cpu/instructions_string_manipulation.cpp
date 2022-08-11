#include "core/arch.hpp"
#include "core/instructions.hpp"

//TODO: Is there a case where the rep prefix isn't the first ?
static FORCE_INLINE void REP_OPERATIONS(void)
{
	cpu.cx -= 1;

	//REPE (or REPZ) have the value 0xF3 while REPNE (or REPNZ) have the value 0xF2
	const bool prefixIsREPx = cpu.biu.instructionBufferQueue[0] & 0b1;
	const bool REPNxEndCondition = !prefixIsREPx && cpu.getFlagStatus(CPU::ZERRO);
	const bool REPEndCondition = cpu.cx == 0;
	
	cpu.eu.repInstructionFinished = REPEndCondition || REPNxEndCondition;
}

static FORCE_INLINE void REPE_OPERATIONS(void)
{
	cpu.cx -= 1;

	//REPE (or REPZ) have the value 0xF3 while REPNE (or REPNZ) have the value 0xF2
	const bool prefixIsREPx = cpu.biu.instructionBufferQueue[0] & 0b1;
	const bool REPxEndCondition = prefixIsREPx && !cpu.getFlagStatus(CPU::ZERRO);
	const bool REPNxEndCondition = !prefixIsREPx && cpu.getFlagStatus(CPU::ZERRO);
	const bool REPEndCondition = cpu.cx == 0;
	
	cpu.eu.repInstructionFinished = REPEndCondition || REPxEndCondition || REPNxEndCondition;
}

static FORCE_INLINE void MOVS_OPERATIONS(void)
{
	const unsigned int srcAddr = cpu.genAddress(cpu.ds,cpu.si);
	const unsigned int destAddr = cpu.genAddress(cpu.es ,cpu.di);
	const bool decrement = cpu.getFlagStatus(CPU::DIR);

	if (cpu.eu.operandSizeWord)
	{
		cpu.biu.writeWord(destAddr,cpu.biu.readWord(srcAddr));
		cpu.si = decrement ? (cpu.si - 2) : (cpu.si + 2);
		cpu.di = decrement ? (cpu.di - 2) : (cpu.di + 2);
	}
	else
	{
		cpu.biu.writeByte(destAddr,cpu.biu.readByte(srcAddr));
		cpu.si = decrement ? (cpu.si - 1) : (cpu.si + 1);
		cpu.di = decrement ? (cpu.di - 1) : (cpu.di + 1);
	}
}
void MOVS(void) { MOVS_OPERATIONS(); }
void REP_MOVS(void)
{
	MOVS_OPERATIONS();
	REP_OPERATIONS();
}

static FORCE_INLINE void CMPS_OPERATIONS(void){
	const unsigned int src1 = cpu.genAddress(cpu.ds,cpu.si);
	const unsigned int src2 = cpu.genAddress(cpu.es,cpu.di);
	const unsigned int temp = src1 - src2;
	const bool decrement = cpu.getFlagStatus(CPU::DIR);
	cpu.updateStatusFlags(temp,cpu.eu.operandSizeWord);
	
	if (cpu.eu.operandSizeWord)
	{
		cpu.si = decrement ? (cpu.si - 2) : (cpu.si + 2);
		cpu.di = decrement ? (cpu.di - 2) : (cpu.di + 2);
	}
	else
	{
		cpu.si = decrement ? (cpu.si - 1) : (cpu.si + 1);
		cpu.di = decrement ? (cpu.di - 1) : (cpu.di + 1);
	}
}
void CMPS (void) { CMPS_OPERATIONS(); }
void REP_CMPS (void)
{
	CMPS_OPERATIONS();
	REPE_OPERATIONS();
}

void FORCE_INLINE SCAS_OPERATIONS (void)
{
	const unsigned int destAddr = cpu.genAddress(cpu.ds,cpu.di);
	const bool decrement = cpu.getFlagStatus(CPU::DIR);
	unsigned int temp;

	if (cpu.eu.operandSizeWord)
	{
		temp = cpu.ax - cpu.biu.readWord(destAddr);
		cpu.di = decrement ? (cpu.di - 2) : (cpu.di + 2);
	}
	else
	{
		temp = cpu.al - cpu.biu.readByte(destAddr);
		cpu.di = decrement ? (cpu.di - 1) : (cpu.di + 1);
	}

	cpu.updateStatusFlags(temp,cpu.eu.operandSizeWord);
}
void SCAS(void){ SCAS_OPERATIONS(); }
void REP_SCAS(void)
{
	SCAS_OPERATIONS();
	REPE_OPERATIONS();
}

static FORCE_INLINE void LODS_OPERATIONS (void)
{
	const unsigned int srcAddr = cpu.genAddress(cpu.ds,cpu.si);
	const bool decrement = cpu.getFlagStatus(CPU::DIR);

	if (cpu.eu.operandSizeWord)
	{
		cpu.ax = cpu.biu.readWord(srcAddr);
		cpu.si = decrement ? (cpu.si - 2) : (cpu.si + 2);
	}
	else
	{
		cpu.al = cpu.biu.readByte(srcAddr);
		cpu.si = decrement ? (cpu.si - 1) : (cpu.si + 1);
	}
}
void LODS(void) { LODS_OPERATIONS(); }
void REP_LODS(void)
{
	LODS_OPERATIONS();
	REP_OPERATIONS();
}

static void FORCE_INLINE STOS_OPERATIONS (void)
{
	const unsigned int destAddr = cpu.genAddress(cpu.ds,cpu.di);
	const bool decrement = cpu.getFlagStatus(CPU::DIR);

	if (cpu.eu.operandSizeWord)
	{
		cpu.biu.writeWord(destAddr,cpu.ax);
		cpu.di = decrement ? (cpu.di - 2) : (cpu.di + 2);
	}
	else
	{
		cpu.biu.writeByte(destAddr,cpu.al);
		cpu.di = decrement ? (cpu.di - 1) : (cpu.di + 1);
	}
}
void STOS(void) { STOS_OPERATIONS(); }
void REP_STOS(void)
{
	STOS_OPERATIONS();
	REP_OPERATIONS();
}
