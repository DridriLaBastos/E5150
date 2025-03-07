#include "core/instructions.hpp"

bool GetJECondition(E5150::Intel8088* cpu)
{ return cpu->GetFlags(E5150::Intel8088::ECpuFlags::ZERO); }

bool GetJLCondition(E5150::Intel8088* cpu)
{ return cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN) != cpu->GetFlags(E5150::Intel8088::ECpuFlags::OVER); }

bool GetJLECondition(E5150::Intel8088* cpu)
{ return GetJECondition(cpu) || GetJLCondition(cpu); }

bool GetJBCondition(E5150::Intel8088* cpu)
{ return cpu->GetFlags(E5150::Intel8088::ECpuFlags::CARRY); }

bool GetJBECondition(E5150::Intel8088* cpu)
{ return GetJBCondition(cpu) || GetJECondition(cpu);  }

bool GetJPCondition(E5150::Intel8088* cpu)
{ return cpu->GetFlags(E5150::Intel8088::ECpuFlags::PARITY); }

bool GetJOCondition(E5150::Intel8088* cpu)
{ return cpu->GetFlags(E5150::Intel8088::ECpuFlags::OVER); }

bool GetJSCondition(E5150::Intel8088* cpu)
{ return cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN); }

bool GetJNZCondition(E5150::Intel8088* cpu)
{ return !cpu->GetFlags(E5150::Intel8088::ECpuFlags::ZERO); }

bool GetJNLCondition(E5150::Intel8088* cpu)
{ return !GetJLCondition(cpu); }

bool GetJNLECondition(E5150::Intel8088* cpu)
{ return GetJNLCondition(cpu) || GetJECondition(cpu); }

bool GetJNBCondition(E5150::Intel8088* cpu)
{ return !GetJBCondition(cpu); }

bool GetJNBECondition(E5150::Intel8088* cpu)
{ return GetJNBCondition(cpu) || GetJECondition(cpu); }

bool GetJNPCondition(E5150::Intel8088* cpu)
{ return !GetJPCondition(cpu); }

bool GetJNSCondition(E5150::Intel8088* cpu)
{ return !GetJSCondition(cpu); }
