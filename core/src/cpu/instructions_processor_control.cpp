#include "core/arch.hpp"
#include "core/instructions.hpp"

void CLC (E5150::Intel8088* cpu) { cpu->ClearFlags(E5150::Intel8088::ECpuFlags::CARRY); }
void CMC (E5150::Intel8088* cpu) { cpu->ToggleFlags(E5150::Intel8088::ECpuFlags::CARRY); }
void STC (E5150::Intel8088* cpu) { cpu->SetFlags(E5150::Intel8088::ECpuFlags::CARRY); }

void CLD (E5150::Intel8088* cpu) { cpu->ClearFlags(E5150::Intel8088::ECpuFlags::DIR); }
void STD (E5150::Intel8088* cpu) { cpu->SetFlags(E5150::Intel8088::ECpuFlags::DIR); }

void _CLI (E5150::Intel8088* cpu) { cpu->ClearFlags(E5150::Intel8088::ECpuFlags::INTF); }
void STI (E5150::Intel8088* cpu) { cpu->SetFlags(E5150::Intel8088::ECpuFlags::INTF); }

void HLT (E5150::Intel8088* cpu) { cpu->hlt(); }

void NOP() { }

/**
 * A | B | XOR
 * 0   0    0
 * 0   1    1
 * 1   0    1
 * 1   1    0
*/