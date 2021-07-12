#include "8086.hpp"

void CPU::CLC (void) { clearFlags(CARRY); }
void CPU::STC (void) { setFlags(CARRY); }

void CPU::CLD (void) { clearFlags(DIR); }
void CPU::STD (void) { setFlags(DIR); }

void CPU::CLI (void) { clearFlags(INTF); }
void CPU::STI (void) { setFlags(INTF); }

void CPU::HLT (void) { hlt = true; }