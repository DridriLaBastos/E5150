#include "arch.hpp"
#include "instructions.hpp"

void NOT()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));

	if (op_name == XED_OPERAND_REG0)
		cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name), ~cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name)));
	else
	{
		const unsigned int addr = cpu.genEA();
		cpu.biu.writeByte(addr, ~cpu.biu.readByte(addr));

		if (xed_decoded_inst_get_memory_operand_length(&cpu.eu.decodedInst, 0) == 2)
			cpu.biu.writeByte(addr+1, ~cpu.biu.readByte(addr+1));
	}
}

void SHL	(void){}
void SHR	(void){}
void SAR	(void){}
void ROL	(void){}
void ROR	(void){}
void RCL	(void){}
void RCR	(void){}
void AND	(void){}
void TEST	(void){}
void OR		(void){}
void XOR	(void){}