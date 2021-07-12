#include "8086.hpp"
#include "instructions.hpp"

void NEAR_CALL(CPU& cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.decodedInst), 0));

	cpu.push(cpu.ip);

	switch (op_name)
	{   
		case XED_OPERAND_REG0:
			cpu.ip = cpu.readReg(xed_decoded_inst_get_reg(&cpu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_RELBR:
			cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
			break;
		
		case XED_OPERAND_MEM0:
			cpu.ip = cpu.readWord(cpu.genEA());
			break;
	}
}

void FAR_CALL(CPU& cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_PTR:
			cpu.far_call(xed_decoded_inst_get_unsigned_immediate(&cpu.decodedInst),
					xed_decoded_inst_get_branch_displacement(&cpu.decodedInst));
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

void NEAR_JMP(CPU& cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
			cpu.ip = cpu.readWord(cpu.genEA());
			break;

		case XED_OPERAND_REG0:
			cpu.ip = cpu.readReg(xed_decoded_inst_get_reg(&cpu.decodedInst, op_name));
			break;

		case XED_OPERAND_RELBR:
			cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
			break;
	}
}

void FAR_JMP(CPU& cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.decodedInst), 0));

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
			cpu.ip = xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
			cpu.regs[CPU::CS] = xed_decoded_inst_get_unsigned_immediate(&cpu.decodedInst);
			break;
	}
}

void NEAR_RET(CPU& cpu)
{
	cpu.ip = cpu.pop();

	if (xed_decoded_inst_get_length(&cpu.decodedInst) > 1)
		cpu.regs[CPU::SP] += xed_decoded_inst_get_unsigned_immediate(&cpu.decodedInst);
}

void FAR_RET(CPU& cpu)
{
	cpu.far_ret();

	if (xed_decoded_inst_get_length(&cpu.decodedInst) > 1)
		cpu.regs[CPU::SP] += xed_decoded_inst_get_unsigned_immediate(&cpu.decodedInst);
}

void JZ(CPU& cpu)
{
	if (cpu.getFlagStatus(CPU::ZERRO))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
}

void JL(CPU& cpu)
{
	if (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
}

void JLE(CPU& cpu)
{
	if (cpu.getFlagStatus(CPU::ZERRO) || (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER)))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
}

void JNZ(CPU& cpu)
{
	if (!cpu.getFlagStatus(CPU::ZERRO))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
}

void JNL(CPU& cpu)
{
	if (cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
}

void JNLE(CPU& cpu)
{
	if (!(cpu.getFlagStatus(CPU::ZERRO) || (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER))))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
}

void LOOP(CPU& cpu)
{
	if (cpu.regs[CPU::CX]-- != 0)
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
}

void JCXZ(CPU& cpu)
{
	if (cpu.regs[CPU::CX].x == 0)
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
}

void INT(CPU& cpu)
{
	cpu.intr_v = (uint8_t)xed_decoded_inst_get_unsigned_immediate(&cpu.decodedInst);
	cpu.interrupt(); //interrupt takes care of cpu.pushing flags and clearing IF
}

void IRET (CPU& cpu)
{
	cpu.far_ret();
	cpu.flags = cpu.pop();
}