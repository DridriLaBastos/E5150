#include "arch.hpp"
#include "instructions.hpp"

void CALL_NEAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	
	cpu.push(cpu.ip);
	cpu.biu.resetInstructionBufferQueue();

	switch (op_name)
	{   
		case XED_OPERAND_REG0:
			cpu.ip = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_RELBR:
			cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			break;
		
		case XED_OPERAND_MEM0:
			cpu.ip = cpu.biu.EURequestReadWord(cpu.genEA());
			break;
	}
}

void CALL_FAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	cpu.biu.resetInstructionBufferQueue();

	switch (op_name)
	{
		case XED_OPERAND_PTR:
			cpu.far_call(xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst),
					xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned far_addr_location = cpu.genEA();
			
			cpu.far_call(cpu.biu.EURequestReadWord(far_addr_location),
					 cpu.biu.EURequestReadWord(far_addr_location + 2));
			break;
		}
	}
}

void JMP_NEAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	cpu.biu.resetInstructionBufferQueue();

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
			cpu.ip = cpu.biu.EURequestReadWord(cpu.genEA());
			break;

		case XED_OPERAND_REG0:
			cpu.ip = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;

		case XED_OPERAND_RELBR:
			cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			break;
	}
}

void JMP_FAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	cpu.biu.resetInstructionBufferQueue();

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
		{
			const unsigned far_addr_location = cpu.genEA();
			
			cpu.regs[CPU::CS] = cpu.biu.EURequestReadWord(far_addr_location);
			cpu.ip = cpu.biu.EURequestReadWord(far_addr_location + 2);
			break;
		}

		case XED_OPERAND_PTR:
			cpu.ip = xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			cpu.regs[CPU::CS] = xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
			break;
	}
}

void RET_NEAR()
{
	cpu.ip = cpu.pop();
	cpu.biu.resetInstructionBufferQueue();

	if (xed_decoded_inst_get_length(&cpu.eu.decodedInst) > 1)
		cpu.regs[CPU::SP] += xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
}

void RET_FAR()
{
	cpu.far_ret();
	cpu.biu.resetInstructionBufferQueue();

	if (xed_decoded_inst_get_length(&cpu.eu.decodedInst) > 1)
		cpu.regs[CPU::SP] += xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
}

void JZ()
{
	cpu.biu.resetInstructionBufferQueue();
	if (cpu.getFlagStatus(CPU::ZERRO))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JL()
{
	cpu.biu.resetInstructionBufferQueue();
	if (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JLE()
{
	cpu.biu.resetInstructionBufferQueue();
	if (cpu.getFlagStatus(CPU::ZERRO) || (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER)))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JNZ()
{
	cpu.biu.resetInstructionBufferQueue();
	if (!cpu.getFlagStatus(CPU::ZERRO))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JNL()
{
	cpu.biu.resetInstructionBufferQueue();
	if (cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JNLE()
{
	cpu.biu.resetInstructionBufferQueue();
	if (!(cpu.getFlagStatus(CPU::ZERRO) || (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER))))
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void LOOP()
{
	cpu.biu.resetInstructionBufferQueue();
	if (cpu.regs[CPU::CX]-- != 0)
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void JCXZ()
{
	cpu.biu.resetInstructionBufferQueue();
	if (cpu.regs[CPU::CX].x == 0)
		cpu.ip += xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
}

void INT()
{
	cpu.biu.resetInstructionBufferQueue();
	cpu.intr_v = (uint8_t)xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
	cpu.interrupt(); //interrupt takes care of cpu.pushing flags and clearing IF
}

void IRET ()
{
	cpu.biu.resetInstructionBufferQueue();
	cpu.far_ret();
	cpu.flags = cpu.pop();
}
