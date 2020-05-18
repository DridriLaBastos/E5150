#include "8086.hpp"

void CPU::NOT()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&m_decoded_inst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));

	if (op_name == XED_OPERAND_REG0)
		write_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name), ~read_reg(xed_decoded_inst_get_reg(&m_decoded_inst, op_name)));
	else
	{
		const unsigned int addr = genEA();
		m_ram.m_ram[addr] = ~m_ram.m_ram[addr];

		if (xed_decoded_inst_get_memory_operand_length(&m_decoded_inst, 0) == 2)
			m_ram.m_ram[addr + 1] = ~m_ram.m_ram[addr + 1];
	}
}