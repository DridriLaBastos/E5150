#include "core/arch.hpp"
#include "core/instructions.hpp"

void CALL_NEAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	cpu.push(cpu.regs.ip);

	switch (op_name)
	{   
		case XED_OPERAND_REG0:
			cpu.regs.ip = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_RELBR:
			cpu.regs.ip = cpu.regs.ip + xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			break;
		
		case XED_OPERAND_MEM0:
			cpu.regs.ip = cpu.biu.readWord(cpu.eu.EAddress);
			break;
	}

	cpu.biu.endControlTransferInstruction();
}

void CALL_FAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_PTR:
			cpu.eu.farCall( xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst),
							xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst));
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
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
			cpu.regs.ip = cpu.biu.readWord(cpu.eu.EAddress);
			break;

		case XED_OPERAND_REG0:
			cpu.regs.ip = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;

		case XED_OPERAND_RELBR:
			cpu.regs.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			break;
	}

	cpu.biu.endControlTransferInstruction();
}

void JMP_FAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
		{
			const unsigned far_addr_location = cpu.eu.EAddress;
			
			cpu.regs.cs = cpu.biu.readWord(far_addr_location);
			cpu.regs.ip = cpu.biu.readWord(far_addr_location + 2);
			break;
		}

		case XED_OPERAND_PTR:
			cpu.regs.cs = xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
			cpu.regs.ip = xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			break;
	}

	cpu.biu.endControlTransferInstruction();
}

void RET_NEAR()
{
	cpu.regs.ip = cpu.pop();
	cpu.regs.cs = cpu.regs.cs;

	if (xed_decoded_inst_get_length(&cpu.eu.decodedInst) > 1)
		cpu.regs.sp += xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
	
	cpu.biu.endControlTransferInstruction();
}

void RET_FAR()
{
	cpu.eu.farRet();

	if (xed_decoded_inst_get_length(&cpu.eu.decodedInst) > 1)
		cpu.regs.sp += xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
	
	cpu.biu.endControlTransferInstruction();
}

static FORCE_INLINE void JMP_NEAR_ON_CONDITION(const bool condition)
{
	if(condition)
	{ cpu.regs.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst); }
	cpu.biu.endControlTransferInstruction(condition);
}

//TODO: Wrong clock cycles for JO (at least)
void JZ()  { JMP_NEAR_ON_CONDITION(cpu.getFlagStatus(CPU::ZERRO)); }
void JL()  { JMP_NEAR_ON_CONDITION(cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER)); }
void JLE() { JMP_NEAR_ON_CONDITION(cpu.getFlagStatus(CPU::ZERRO) || (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER))); }
void JB()  { JMP_NEAR_ON_CONDITION(cpu.getFlagStatus(CPU::CARRY)); }
void JBE() { JMP_NEAR_ON_CONDITION(cpu.getFlagStatus(CPU::CARRY) || cpu.getFlagStatus(CPU::ZERRO)); }
void JP()  { JMP_NEAR_ON_CONDITION(cpu.getFlagStatus(CPU::PARRITY)); }
void JO()  { JMP_NEAR_ON_CONDITION(cpu.getFlagStatus(CPU::OVER)); }
void JS()  { JMP_NEAR_ON_CONDITION(cpu.getFlagStatus(CPU::SIGN)); }
void JNZ() { JMP_NEAR_ON_CONDITION(!cpu.getFlagStatus(CPU::ZERRO)); }
void JNL() { JMP_NEAR_ON_CONDITION(cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER)); }
void JNLE() { JMP_NEAR_ON_CONDITION(!cpu.getFlagStatus(CPU::ZERRO) && (cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER))); }
void JNB() { JMP_NEAR_ON_CONDITION(!cpu.getFlagStatus(CPU::CARRY)); }
void JNBE() { JMP_NEAR_ON_CONDITION(!(cpu.getFlagStatus(CPU::CARRY) || cpu.getFlagStatus(CPU::ZERRO))); }
void JNP() { JMP_NEAR_ON_CONDITION(!cpu.getFlagStatus(CPU::PARRITY)); }
void JNS() { JMP_NEAR_ON_CONDITION(!cpu.getFlagStatus(CPU::SIGN)); }

void LOOP()   { cpu.regs.cx -= 1; JMP_NEAR_ON_CONDITION(cpu.regs.cx != 0); }
void LOOPZ()  { cpu.regs.cx -= 1; JMP_NEAR_ON_CONDITION((cpu.regs.cx != 0) && cpu.getFlagStatus(CPU::ZERRO)); }
void LOOPNZ() { cpu.regs.cx -= 1; JMP_NEAR_ON_CONDITION((cpu.regs.cx != 0) && !cpu.getFlagStatus(CPU::ZERRO)); }

void JCXZ() { JMP_NEAR_ON_CONDITION(cpu.regs.cx == 0); }

void IRET ()
{
	cpu.eu.farRet();
	cpu.regs.flags = cpu.pop();
	cpu.biu.endControlTransferInstruction();
	cpu.iretDelay();
}
