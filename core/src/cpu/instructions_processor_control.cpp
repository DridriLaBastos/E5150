#include "arch.hpp"
#include "instructions.hpp"

void CLC () { cpu.clearFlags(CPU::CARRY); }
void CMC () { cpu.toggleFlags(CPU::CARRY); }
void STC () { cpu.setFlags(CPU::CARRY); }

void CLD () { cpu.clearFlags(CPU::DIR); }
void STD () { cpu.setFlags(CPU::DIR); }

void CLI () { cpu.clearFlags(CPU::INTF); }
void STI () { cpu.setFlags(CPU::INTF); }

void HLT () { cpu.hlt = true; }

void NOP() { }