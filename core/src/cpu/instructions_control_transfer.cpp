#include "arch.hpp"
#include "instructions.hpp"

#define JMP_SHORT_ON_CONDITION(COND) if(COND){\
										cpu.biu.startControlTransferInstruction();\
										cpu.eu.newIP = cpu.ip + xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);\
										cpu.eu.newCS = cpu.cs;\
										cpu.eu.newFetchAddress = true;}

void CALL_NEAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	cpu.eu.push(cpu.ip);
	cpu.biu.startControlTransferInstruction();

	switch (op_name)
	{   
		case XED_OPERAND_REG0:
			cpu.eu.newIP = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;
		
		case XED_OPERAND_RELBR:
			cpu.eu.newIP = cpu.ip + xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			break;
		
		case XED_OPERAND_MEM0:
			cpu.eu.newIP = cpu.biu.EURequestReadWord(cpu.genEA());
			break;
	}

	cpu.eu.newCS = cpu.cs;
	cpu.eu.newFetchAddress = true;
}

void CALL_FAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	cpu.biu.startControlTransferInstruction();

	switch (op_name)
	{
		case XED_OPERAND_PTR:
			cpu.eu.farCall( xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst),
							xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst));
			break;
		
		case XED_OPERAND_MEM0:
		{
			const unsigned far_addr_location = cpu.genEA();
			cpu.eu.farCall( cpu.biu.EURequestReadWord(far_addr_location),
							cpu.biu.EURequestReadWord(far_addr_location + 2));
			break;
		}
	}
}

void JMP_NEAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	cpu.biu.startControlTransferInstruction();

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
			cpu.eu.newIP = cpu.biu.EURequestReadWord(cpu.genEA());
			break;

		case XED_OPERAND_REG0:
			cpu.eu.newIP = cpu.readReg(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
			break;

		case XED_OPERAND_RELBR:
			cpu.eu.newIP = cpu.ip + xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			break;
	}

	cpu.eu.newCS = cpu.cs;
	cpu.eu.newFetchAddress = true;
}

void JMP_FAR()
{
	const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(xed_decoded_inst_inst(&cpu.eu.decodedInst), 0));
	cpu.biu.startControlTransferInstruction();

	switch (op_name)
	{
		case XED_OPERAND_MEM0:
		{
			const unsigned far_addr_location = cpu.genEA();
			
			cpu.eu.newCS = cpu.biu.EURequestReadWord(far_addr_location);
			cpu.eu.newIP = cpu.biu.EURequestReadWord(far_addr_location + 2);
			break;
		}

		case XED_OPERAND_PTR:
			cpu.eu.newCS = xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
			cpu.eu.newIP = xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
			break;
	}
	cpu.eu.newFetchAddress = true;
}

void RET_NEAR()
{
	cpu.biu.startControlTransferInstruction();
	cpu.eu.newIP = cpu.eu.pop();
	cpu.eu.newCS = cpu.cs;
	cpu.eu.newFetchAddress = true;

	if (xed_decoded_inst_get_length(&cpu.eu.decodedInst) > 1)
		cpu.regs[CPU::SP] += xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
}

void RET_FAR()
{
	cpu.biu.startControlTransferInstruction();
	cpu.eu.farRet();

	if (xed_decoded_inst_get_length(&cpu.eu.decodedInst) > 1)
		cpu.regs[CPU::SP] += xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
}

void JZ()  { JMP_SHORT_ON_CONDITION(cpu.getFlagStatus(CPU::ZERRO)); }
void JL()  { JMP_SHORT_ON_CONDITION(cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER)); }
void JLE() { JMP_SHORT_ON_CONDITION(cpu.getFlagStatus(CPU::ZERRO) || (cpu.getFlagStatus(CPU::SIGN) != cpu.getFlagStatus(CPU::OVER))); }
void JB()  { JMP_SHORT_ON_CONDITION(cpu.getFlagStatus(CPU::CARRY)); }
void JBE() { JMP_SHORT_ON_CONDITION(cpu.getFlagStatus(CPU::CARRY) || cpu.getFlagStatus(CPU::ZERRO)); }
void JP()  { JMP_SHORT_ON_CONDITION(cpu.getFlagStatus(CPU::PARRITY)); }
void JO()  { JMP_SHORT_ON_CONDITION(cpu.getFlagStatus(CPU::OVER)); }
void JS()  { JMP_SHORT_ON_CONDITION(cpu.getFlagStatus(CPU::SIGN)); }
void JNZ() { JMP_SHORT_ON_CONDITION(!cpu.getFlagStatus(CPU::ZERRO)); }
void JNL() { JMP_SHORT_ON_CONDITION(cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER)); }
void JNLE() { JMP_SHORT_ON_CONDITION(!cpu.getFlagStatus(CPU::ZERRO) && (cpu.getFlagStatus(CPU::SIGN) == cpu.getFlagStatus(CPU::OVER))); }
void JNB() { JMP_SHORT_ON_CONDITION(!cpu.getFlagStatus(CPU::CARRY)); }
void JNBE() { JMP_SHORT_ON_CONDITION(!(cpu.getFlagStatus(CPU::CARRY) || cpu.getFlagStatus(CPU::ZERRO))); }
void JNP() { JMP_SHORT_ON_CONDITION(!cpu.getFlagStatus(CPU::PARRITY)); }
void JNS() { JMP_SHORT_ON_CONDITION(!cpu.getFlagStatus(CPU::SIGN)); }

void LOOP()   { JMP_SHORT_ON_CONDITION(cpu.cx-- != 0); }
void LOOPZ()  { JMP_SHORT_ON_CONDITION((cpu.cx-- != 0) && cpu.getFlagStatus(CPU::ZERRO)); }
void LOOPNZ() { JMP_SHORT_ON_CONDITION((cpu.cx-- != 0) && !cpu.getFlagStatus(CPU::ZERRO)); }

void JCXZ() { JMP_SHORT_ON_CONDITION(cpu.cx == 0); }

void INT()
{
	cpu.biu.startControlTransferInstruction();
	cpu.intr_v = (uint8_t)xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst);
	cpu.interrupt(); //interrupt takes care of cpu.pushing flags and clearing IF
}

void INTO(){}

void IRET ()
{
	cpu.biu.startControlTransferInstruction();
	cpu.eu.farRet();
	cpu.flags = cpu.eu.pop();
}
