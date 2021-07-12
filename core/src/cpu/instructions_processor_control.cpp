#include "8086.hpp"
#include "instructions.hpp"

void CLC (CPU& cpu) { cpu.clearFlags(CPU::CARRY); }
void STC (CPU& cpu) { cpu.setFlags(CPU::CARRY); }

void CLD (CPU& cpu) { cpu.clearFlags(CPU::DIR); }
void STD (CPU& cpu) { cpu.setFlags(CPU::DIR); }

void CLI (CPU& cpu) { cpu.clearFlags(CPU::INTF); }
void STI (CPU& cpu) { cpu.setFlags(CPU::INTF); }

void HLT (CPU& cpu) { cpu.hlt = true; }

void NOP(CPU& cpu) { cpu.clockCountDown = 3; }