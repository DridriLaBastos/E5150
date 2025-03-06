#include "util.hpp"
#include "core/arch.hpp"
#include "core/instructions.hpp"

#if 0
void CALL_NEAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu->decodedInst), 0));
	cpu.push(cpu.regs.ip);

	switch (op_name)
	{   
		case XED_OPERAND_REG0:
			cpu.regs.ip = cpu.readReg(xed_decoded_inst_get_reg(&cpu->decodedInst, op_name));
			break;
		
		case XED_OPERAND_RELBR:
			cpu.regs.ip = cpu.regs.ip + xed_decoded_inst_get_branch_displacement(&cpu->decodedInst);
			break;
		
		case XED_OPERAND_MEM0:
			cpu.regs.ip = cpu.biu.readWord(cpu.eu.EAddress);
			break;
	}

	cpu.biu.endControlTransferInstruction();
}

void CALL_FAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu->decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_PTR:
			cpu.eu.farCall( xed_decoded_inst_get_unsigned_immediate(&cpu->decodedInst),
							xed_decoded_inst_get_branch_displacement(&cpu->decodedInst));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned far_addr_location = cpu.eu.EAddress;
			cpu.eu.farCall( cpu.biu.readWord(far_addr_location),
							cpu.biu.readWord(far_addr_location + 2));
			break;
		}
	}

	cpu.biu.endControlTransferInstruction();
}

void JMP_NEAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu->decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
			cpu.regs.ip = cpu.biu.readWord(cpu.eu.EAddress);
			break;

		case XED_OPERAND_REG0:
			cpu.regs.ip = cpu.readReg(xed_decoded_inst_get_reg(&cpu->decodedInst, op_name));
			break;

		case XED_OPERAND_RELBR:
			cpu.regs.ip += xed_decoded_inst_get_branch_displacement(&cpu->decodedInst);
			break;
	}

	cpu.biu.endControlTransferInstruction();
}
#endif
void JMP_FAR(E5150::Intel8088* cpu)
{
	switch (const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu->decodedInst), 0)))
	{
		case XED_OPERAND_MEM0:
		{
			const unsigned far_addr_location = cpu->GenerateEffectiveAddress();
			
			cpu->regs.cs = E5150::Arch::ram.ReadWord(far_addr_location);
			cpu->regs.ip = E5150::Arch::ram.ReadWord(far_addr_location + 2);
			break;
		}

		case XED_OPERAND_PTR:
			cpu->regs.cs = xed_decoded_inst_get_unsigned_immediate(&cpu->decodedInst);
			cpu->regs.ip = xed_decoded_inst_get_branch_displacement(&cpu->decodedInst);
			break;
	}

	cpu->EndControlTransferInstruction();
}

#if 0
void RET_NEAR()
{
	cpu.regs.ip = cpu.pop();
	cpu.regs.cs = cpu.regs.cs;

	if (xed_decoded_inst_get_length(&cpu->decodedInst) > 1)
		cpu.regs.sp += xed_decoded_inst_get_unsigned_immediate(&cpu->decodedInst);
	
	cpu.biu.endControlTransferInstruction();
}

void RET_FAR()
{
	cpu.eu.farRet();

	if (xed_decoded_inst_get_length(&cpu->decodedInst) > 1)
		cpu.regs.sp += xed_decoded_inst_get_unsigned_immediate(&cpu->decodedInst);
	
	cpu.biu.endControlTransferInstruction();
}

#endif

static FORCE_INLINE void JMP_NEAR_ON_CONDITION(const bool condition, E5150::Intel8088* cpu)
{
	if(condition)
	{
		cpu->regs.ip += xed_decoded_inst_get_branch_displacement(&cpu->decodedInst);
	}

	cpu->EndControlTransferInstruction(condition);
}

//TODO: Wrong clock cycles for JO (at least)
void JZ   (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN),cpu); }
void JL   (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(cpu->GetFlags(E5150::Intel8088::ECpuFlags::ZERO) != cpu->GetFlags(E5150::Intel8088::ECpuFlags::OVER),cpu); }
void JLE  (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN) || (cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN) != cpu->GetFlags(E5150::Intel8088::ECpuFlags::OVER)),cpu); }
void JB   (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(cpu->GetFlags(E5150::Intel8088::ECpuFlags::CARRY),cpu); }
void JBE  (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(cpu->GetFlags(E5150::Intel8088::ECpuFlags::CARRY) || cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN),cpu); }
void JP   (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(cpu->GetFlags(E5150::Intel8088::ECpuFlags::PARITY),cpu); }
void JO   (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(cpu->GetFlags(E5150::Intel8088::ECpuFlags::OVER),cpu); }
void JS   (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN),cpu); }
void JNZ  (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(!cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN),cpu); }
void JNL  (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN) == cpu->GetFlags(E5150::Intel8088::ECpuFlags::OVER),cpu); }
void JNLE (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(!cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN) && (cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN) == cpu->GetFlags(E5150::Intel8088::ECpuFlags::OVER)),cpu); }
void JNB  (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(!cpu->GetFlags(E5150::Intel8088::ECpuFlags::CARRY),cpu); }
void JNBE (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(!(cpu->GetFlags(E5150::Intel8088::ECpuFlags::CARRY) || cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN)),cpu); }
void JNP  (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(!cpu->GetFlags(E5150::Intel8088::ECpuFlags::PARITY),cpu); }
void JNS  (E5150::Intel8088* cpu) { JMP_NEAR_ON_CONDITION(!cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN),cpu); }

#if 0
void LOOP()   { cpu.regs.cx -= 1; JMP_NEAR_ON_CONDITION(cpu.regs.cx != 0); }
void LOOPZ()  { cpu.regs.cx -= 1; JMP_NEAR_ON_CONDITION((cpu.regs.cx != 0) && cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN)); }
void LOOPNZ() { cpu.regs.cx -= 1; JMP_NEAR_ON_CONDITION((cpu.regs.cx != 0) && !cpu->GetFlags(E5150::Intel8088::ECpuFlags::SIGN)); }

void JCXZ() { JMP_NEAR_ON_CONDITION(cpu.regs.cx == 0); }

void IRET ()
{
	cpu.eu.farRet();
	cpu.regs.flags = cpu.pop();
	cpu.biu.endControlTransferInstruction();
	cpu.iretDelay();
}
#endif
