#include "arch.hpp"
#include "instructions.hpp"

#define MOVS_OPERATIONS() const unsigned int srcAddr = cpu.genAddress(cpu.ds,cpu.si);\
	const unsigned int destAddr = cpu.genAddress(cpu.ds,cpu.di);\
	const bool decrement = cpu.getFlagStatus(CPU::DIR);\
	if (cpu.eu.operandSizeWord) {\
		cpu.biu.writeWord(destAddr,cpu.biu.readWord(srcAddr));\
		cpu.si = decrement ? (cpu.si - 2) : (cpu.si + 2);\
		cpu.di = decrement ? (cpu.di - 2) : (cpu.di + 2); } else {\
		cpu.biu.writeByte(destAddr,cpu.biu.readByte(srcAddr));\
		cpu.si = decrement ? (cpu.si - 1) : (cpu.si + 1);\
		cpu.di = decrement ? (cpu.di - 1) : (cpu.di + 1); }

#define COMPUTE_REP_INSTRUCTION_END_CONDITION()\
	cpu.cx == 0 || ((cpu.biu.instructionBufferQueue[0] & 1) & !cpu.getFlagStatus(CPU::ZERRO))

void REP_MOVS(void)
{
	MOVS_OPERATIONS();
	cpu.cx -= cpu.eu.operandSizeWord ? 2 : 1;
	cpu.eu.repInstructionFinished = COMPUTE_REP_INSTRUCTION_END_CONDITION();
}

void MOVS(void) { MOVS_OPERATIONS(); }

void REP_CMPS(void) {}
void CMPS (void)
{
	const unsigned int src1 = cpu.genAddress(cpu.ds,cpu.si);
	const unsigned int src2 = cpu.genAddress(cpu.ds,cpu.di);
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

void REP_SCAS(void) {}
void SCAS (void)
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

void REP_LODS(void) {}
void LODS (void)
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

void REP_STOS(void) {}
void STOS (void)
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