#include "8086.hpp"

void CPU::NEAR_CALL()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));

	push(m_ip);

	switch (op_name)
	{   
		case XED_OPERAND_REG0:
			m_ip = read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name));
			break;
		
		case XED_OPERAND_RELBR:
			m_ip += xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
			break;
		
		case XED_OPERAND_MEM0:
			m_ip = readWord(genEA());
			break;
	}
}

void CPU::FAR_CALL()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));

	switch (op_name)
	{
		case XED_OPERAND_PTR:
			far_call(xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst),
					xed_decoded_inst_get_branch_displacement(&m_decoded_inst));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned far_addr_location = genEA();
			
			far_call(readWord(far_addr_location),
					 readWord(far_addr_location + 2));
			break;
		}
	}
}

void CPU::NEAR_JMP()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
			m_ip = readWord(genEA());
			break;

		case XED_OPERAND_REG0:
			m_ip = read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name));
			break;

		case XED_OPERAND_RELBR:
			m_ip += xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
			break;
	}
}

void CPU::FAR_JMP()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&m_decoded_inst), 0));

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
		{
			const unsigned far_addr_location = genEA();
			
			m_regs[CS] = readWord(far_addr_location);
			m_ip = readWord(far_addr_location + 2);
			break;
		}

		case XED_OPERAND_PTR:
			m_ip = xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
			m_regs[CS] = xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst);
			break;
	}
}

void CPU::NEAR_RET()
{
	m_ip = pop();

	if (xed_decoded_inst_get_length(&m_decoded_inst) > 1)
		m_regs[SP] += xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst);
}

void CPU::FAR_RET()
{
	far_ret();

	if (xed_decoded_inst_get_length(&m_decoded_inst) > 1)
		m_regs[SP] += xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst);
}

void CPU::JZ()
{
	if (getFlagStatus(ZERRO))
		m_ip += xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
}

void CPU::JL()
{
	if (getFlagStatus(SIGN) != getFlagStatus(OVER))
		m_ip += xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
}

void CPU::JLE()
{
	if (getFlagStatus(ZERRO) || (getFlagStatus(SIGN) != getFlagStatus(OVER)))
		m_ip += xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
}

void CPU::JNZ()
{
	if (!getFlagStatus(ZERRO))
		m_ip += xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
}

void CPU::JNL()
{
	if (getFlagStatus(SIGN) == getFlagStatus(OVER))
		m_ip += xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
}

void CPU::JNLE()
{
	if (!(getFlagStatus(ZERRO) || (getFlagStatus(SIGN) != getFlagStatus(OVER))))
		m_ip += xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
}

void CPU::LOOP()
{
	if (m_regs[CX]-- != 0)
		m_ip += xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
}

void CPU::JCXZ()
{
	if (m_gregs[CX].x == 0)
		m_ip += xed_decoded_inst_get_branch_displacement(&m_decoded_inst);
}

void CPU::INT(void)
{
	intr_v = (uint8_t)xed_decoded_inst_get_unsigned_immediate(&m_decoded_inst);
	interrupt(); //interrupt takes care of pushing flags and clearing IF
}

void CPU::IRET (void)
{
	far_ret();
	m_flags = pop();
}