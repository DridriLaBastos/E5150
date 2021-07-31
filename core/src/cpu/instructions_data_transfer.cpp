#include "arch.hpp"
#include "8086.hpp"
#include "instructions.hpp"

void MOV(CPU& _cpu)
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
			move_v = cpu.readWord(cpu.genEA());
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
			const unsigned int write_addr = cpu.genEA();

			if (xed_decoded_inst_get_memory_operand_length(&cpu.eu.decodedInst, 0) == 1)
				cpu.writeByte(write_addr, (uint8_t)move_v);
			else
				cpu.writeWord(write_addr, move_v);
			
			break;
		}
	}
}

void PUSH (CPU& _cpu)
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));

	switch (op_name)
	{
		case XED_OPERAND_REG0:
			cpu.push(cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst,op_name)));
			break;

		case XED_OPERAND_MEM0:
		{
			const unsigned int dataToPushAddr = cpu.genEA();
			cpu.push(cpu.readWord(dataToPushAddr));
		}
		break;
	}
}

void POP (CPU& _cpu)
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

void XCHG(CPU& _cpu)
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
			const unsigned int addr = cpu.genEA();
			const unsigned int value2 = cpu.readWord(addr);
			cpu.write_reg(register_xchg, value2);

			if (xed_decoded_inst_get_memory_operand_length(&cpu.eu.decodedInst, 0) == 1)
				cpu.writeByte(addr, value1);
			else
				cpu.writeWord(addr, value1);
		}
	}
}

void IN(CPU& _cpu)
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));

	uint16_t iaddr = 0;

	if (op_name == XED_OPERAND_IMM0)
		iaddr = (uint16_t)xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
	else 
		iaddr = cpu.dx;

	cpu.ax = ports.read(iaddr);

	if (xed_decoded_inst_get_reg(&cpu.eu.decodedInst, xed_operand_name(xed_inst_operand(inst, 1))) == XED_REG_AX)
		cpu.ax = ports.read(iaddr + 1);
}

void OUT(CPU& _cpu)
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, 0));

	uint16_t oaddr = 0;

	if (op_name == XED_OPERAND_IMM0)
		oaddr = (uint16_t)xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
	else
		oaddr = cpu.dx;
		
	ports.write(oaddr, cpu.ax);

	if (xed_decoded_inst_get_reg(&cpu.eu.decodedInst, xed_operand_name(xed_inst_operand(inst, 1))) == XED_REG_AX)
		ports.write(oaddr + 1, cpu.ax);
}

void XLAT (CPU& _cpu)
{ cpu.ax = cpu.readByte(cpu.genAddress(cpu.ds, cpu.bx + cpu.ax)); }

void LEA(CPU& _cpu)
{
	const xed_operand_enum_t op_name0 = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	const xed_operand_enum_t op_name1 = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 1));

	cpu.write_reg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name0), cpu.genEA());
}

void LDS(CPU& _cpu)
{
	//TODO: Implement LDS
}

void LES(CPU& _cpu)
{
	//TODO: Implement LES
}

void LAHF (CPU& _cpu)
{ cpu.ax = (cpu.flags & 0b11010101)|0b10; }

void SAHF (CPU& _cpu)
{ cpu.setFlags(cpu.ax & (0b11010101)); }

void PUSHF (CPU& _cpu)
{ cpu.push(cpu.flags); }

void POPF (CPU& _cpu)
{ cpu.flags = cpu.pop(); }
