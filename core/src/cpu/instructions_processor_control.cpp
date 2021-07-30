#include "8086.hpp"
#include "instructions.hpp"

void CLC (CPU& _cpu) { cpu.clearFlags(CPU::CARRY); }
void STC (CPU& _cpu) { cpu.setFlags(CPU::CARRY); }

void CLD (CPU& _cpu) { cpu.clearFlags(CPU::DIR); }
void STD (CPU& _cpu) { cpu.setFlags(CPU::DIR); }

void CLI (CPU& _cpu) { cpu.clearFlags(CPU::INTF); }
void STI (CPU& _cpu) { cpu.setFlags(CPU::INTF); }

void HLT (CPU& _cpu) { cpu.hlt = true; }

void NOP(CPU& _cpu) { cpu.clockCountDown = 3; }