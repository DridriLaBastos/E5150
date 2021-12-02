#include "arch.hpp"
#include "8086.hpp"
#include "instructions.hpp"

void MOV()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);

	xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 1));

	uint16_t move_v = 0;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			move_v = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;

		case XED_OPERAND_REG1:
			move_v = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_IMM0:
			move_v = xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
			break;

		case XED_OPERAND_MEM0:
			move_v = cpu.biu.readWord(cpu.eu.EAAddress);
			break;
	}

	op_name = xed_operand_name(xed_inst_operand(inst, 0));

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name), move_v);
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int write_addr = cpu.eu.EAAddress;

			if (xed_decoded_inst_get_memory_operand_length(&cpu.eu.decodedInst, 0) == 1)
				cpu.biu.writeByte(write_addr, (uint8_t)move_v);
			else
				cpu.biu.writeWord(write_addr, move_v);
			
			break;
		}
	}
}

void PUSH ()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			cpu.push(cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name)));
			break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int dataToPushAddr = cpu.eu.EAAddress;
			cpu.push(cpu.biu.readWord(dataToPushAddr));
		}
		break;
	}
}

void POP ()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t tmp_reg = xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name);

			cpu.write_reg(tmp_reg, cpu.pop());

			if (tmp_reg == XED_REG_SP)
				cpu.sp += 2;

			break;
		}

	}
}

void XCHG()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));
	// The second operand is always a register
	const xed_reg_enum_t register_xchg = xed_decoded_inst_get_reg(&cpu.eu.decodedInst, xed_operand_name(xed_inst_operand(inst, 1)));

	const uint16_t value1 = cpu.readReg(register_xchg);
	uint16_t value2;

	switch (op_name)
	{
		case XED_OPERAND_REG0:
		{
			const xed_reg_enum_t reg1 = xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name);
			cpu.write_reg(register_xchg, cpu.readReg(reg1));
			cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name), value1);

		#ifdef DEBUG_BUILD
			if (!E5150::Util::_stop)
			{
				if ((register_xchg == XED_REG_BX) && (reg1 == XED_REG_BX))
				{
					debug<1>("CPU: Breakpoint");
					E5150::Util::_stop = true;
				}
			}
		#endif
		} break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned int addr = cpu.eu.EAAddress;
			const unsigned int value2 = cpu.biu.readWord(addr);
			cpu.write_reg(register_xchg, value2);

			if (xed_decoded_inst_get_memory_operand_length(&cpu.eu.decodedInst, 0) == 1)
				cpu.biu.writeByte(addr, value1);
			else
				cpu.biu.writeWord(addr, value1);
		}
	}
}

void IN()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));

	uint16_t iaddr = 0;

	if (op_name == XED_OPERAND_IMM0)
		iaddr = (uint16_t)xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
	else 
		iaddr = cpu.dx;

	if (cpu.eu.operandSizeWord)
		cpu.ax = cpu.biu.inWord(iaddr);
	else
		cpu.al = cpu.biu.inByte(iaddr);
}

void OUT()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));

	uint16_t oaddr = 0;

	if (op_name == XED_OPERAND_IMM0)
		oaddr = (uint16_t)xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
	else
		oaddr = cpu.dx;
	
	if (cpu.eu.operandSizeWord)
		cpu.biu.outWord(oaddr,cpu.ax);
	else
		cpu.biu.outByte(oaddr,cpu.al);
}

void XLAT ()
{ cpu.ax = cpu.biu.readByte(cpu.genAddress(cpu.ds, cpu.bx + cpu.ax)); }

void LEA()
{
	const xed_operand_enum_t op_name0 = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 1));

	cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name0), cpu.eu.EAAddress);
}

void LDS()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	const unsigned int addr = cpu.eu.EAAddress;
	cpu.ds = cpu.biu.readWord(addr);
	cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name),cpu.biu.readWord(addr+2));
}

void LES()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	const unsigned int addr = cpu.eu.EAAddress;
	cpu.es = cpu.biu.readWord(addr);
	cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name),cpu.biu.readWord(addr+2));
}

static const unsigned int statusFlagsMask = CPU::SIGN | CPU::ZERRO | CPU::A_CARRY | CPU::PARRITY | CPU::CARRY;

void LAHF ()
{ cpu.ah = (cpu.flags & statusFlagsMask) | 0b10; }

void SAHF ()
{ cpu.flags = (cpu.ah & statusFlagsMask) | 0b10; }

void PUSHF ()
{ cpu.push(cpu.flags); }

void POPF ()
{ cpu.flags = cpu.pop(); }
