#include "arch.hpp"
#include "instructions.hpp"

void NEAR_CALL(CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	cpu.push(cpu.ip);

	switch (op_name)
	{   
		case XED_OPERAND_REG0:
			cpu.ip = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_RELBR:
			cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			break;
		
		case XED_OPERAND_MEM0:
			cpu.ip = cpu.readWord(cpu.genEA());
			break;
	}
}

void FAR_CALL(CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_PTR:
			cpu.far_call(xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst),
					xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned far_addr_location = cpu.genEA();
			
			cpu.far_call(cpu.readWord(far_addr_location),
					 cpu.readWord(far_addr_location + 2));
			break;
		}
	}
}

void NEAR_JMP(CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
			cpu.ip = cpu.readWord(cpu.genEA());
			break;

		case XED_OPERAND_REG0:
			cpu.ip = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;

		case XED_OPERAND_RELBR:
			cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			break;
	}
}

void FAR_JMP(CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
		{
			const unsigned far_addr_location = cpu.genEA();
			
			cpu.regs[CPU::CS] = cpu.readWord(far_addr_location);
			cpu.ip = cpu.readWord(far_addr_location + 2);
			break;
		}

		case XED_OPERAND_PTR:
			cpu.ip = xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			cpu.regs[CPU::CS] = xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
			break;
	}
}

void NEAR_RET(CPU& _cpu)
{
	cpu.ip = cpu.pop();

	if (xed_decoded_inst_get_length(&cpu.eu.decodedInst) > 1)
		cpu.regs[CPU::SP] += xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
}

void FAR_RET(CPU& _cpu)
{
	cpu.far_ret();

	if (xed_decoded_inst_get_length(&cpu.eu.decodedInst) > 1)
		cpu.regs[CPU::SP] += xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
}

void JZ(CPU& _cpu)
{
	if (cpu.getFlagStatus(CPU::ZERRO))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JL(CPU& _cpu)
{
	if (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JLE(CPU& _cpu)
{
	if (cpu.getFlagStatus(CPU::ZERRO) || (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER)))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JNZ(CPU& _cpu)
{
	if (!cpu.getFlagStatus(CPU::ZERRO))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JNL(CPU& _cpu)
{
	if (cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JNLE(CPU& _cpu)
{
	if (!(cpu.getFlagStatus(CPU::ZERRO) || (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER))))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void LOOP(CPU& _cpu)
{
	if (cpu.regs[CPU::CX]-- != 0)
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JCXZ(CPU& _cpu)
{
	if (cpu.regs[CPU::CX].x == 0)
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void INT(CPU& _cpu)
{
	cpu.intr_v = (uint8_t)xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
	cpu.interrupt(); //interrupt takes care of cpu.pushing flags and clearing IF
}

void IRET (CPU& _cpu)
{
	cpu.far_ret();
	cpu.flags = cpu.pop();
}