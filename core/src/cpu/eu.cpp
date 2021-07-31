#include "eu.hpp"
#include "arch.hpp"

#include "instructions.hpp"

using namespace E5150::I8086;

EU::EU(): clockCountDown(0) {}

static void printCurrentInstruction()
{
#if defined(SEE_CURRENT_INST) || defined(SEE_ALL)
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	if (!inst)
		return;
	std::cout << std::hex << cpu.cs << ":" << cpu.ip << " (" << cpu.genAddress(cpu.cs,cpu.ip) << ")" << std::dec << ": ";
	//std::cout << xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(&cpu.eu.decodedInst)) << " : length = " << xed_decoded_inst_get_length(&cpu.eu.decodedInst) << std::endl;
	std::cout << xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(&cpu.eu.decodedInst)) << " ";
	unsigned int realOperandPos = 0;
	bool foundPtr = false;

	for (unsigned int i = 0; i < xed_decoded_inst_noperands(&cpu.eu.decodedInst); ++i)
	{
		const xed_operand_enum_t op_name = xed_operand_name(xed_inst_operand(inst, i));
		const xed_operand_visibility_enum_t op_vis = xed_operand_operand_visibility(xed_inst_operand(inst, i));

		if (op_vis != XED_OPVIS_SUPPRESSED)
		{
			if (foundPtr)
			{
				std::cout << ":";
				foundPtr = false;
			}
			else
			{
				if (realOperandPos > 0)
					std::cout << ", ";
			}

			switch (op_name)
			{
			case XED_OPERAND_RELBR:
				std::cout << xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
				break;

			case XED_OPERAND_PTR:
				std::cout << xed_decoded_inst_get_branch_displacement(&cpu.eu.decodedInst);
				foundPtr = true;
				break;

			case XED_OPERAND_REG0:
			case XED_OPERAND_REG1:
			case XED_OPERAND_REG2:
				std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_reg(&cpu.eu.decodedInst, op_name));
				break;

			case XED_OPERAND_IMM0:
			case XED_OPERAND_IMM1:
				std::cout << std::hex << xed_decoded_inst_get_unsigned_immediate(&cpu.eu.decodedInst) << std::dec;
				break;

			//Displaying memory operand woth format SEG:[[BASE +] [INDEX +] DISPLACEMENT ]
			case XED_OPERAND_MEM0:
			{
				const xed_reg_enum_t baseReg = xed_decoded_inst_get_base_reg(&cpu.eu.decodedInst, 0);
				const xed_reg_enum_t indexReg = xed_decoded_inst_get_index_reg(&cpu.eu.decodedInst, 0);
				const int64_t memDisplacement = xed_decoded_inst_get_memory_displacement(&cpu.eu.decodedInst,0);
				std::cout << ((xed_decoded_inst_get_memory_operand_length(&cpu.eu.decodedInst, 0) == 1) ? "BYTE" : "WORD") << " ";
				std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_seg_reg(&cpu.eu.decodedInst, 0)) << ":[";

				if (baseReg != XED_REG_INVALID)
					std::cout << xed_reg_enum_t2str(baseReg);
				
				if (indexReg != XED_REG_INVALID)
				{
					if (baseReg != XED_REG_INVALID)
						std::cout << " + ";
					std::cout << xed_reg_enum_t2str(indexReg);
				}

				if ((indexReg != XED_REG_INVALID) || (baseReg != XED_REG_INVALID))
				{
					if (memDisplacement != 0)
					{
						if (memDisplacement > 0)
							std::cout << " + " << memDisplacement;
						else
							std::cout << " - " << -memDisplacement;
					}
				}
				else
					std::cout << memDisplacement;
				std::cout << "]";
			}	break;

			default:
				break;
			}

			++realOperandPos;
		}
	}
		std::cout << std::endl;
#endif
}

static bool execNonControlTransferInstruction()
{
	bool hasExecutedInstruction = true;

	//TODO: check if this switch needs to be optimized
	switch (xed_decoded_inst_get_iclass(&cpu.eu.decodedInst))
	{
	case XED_ICLASS_MOV:
		MOV(cpu);
		break;

	case XED_ICLASS_PUSH:
		PUSH(cpu);
		break;

	case XED_ICLASS_POP:
		POP(cpu);
		break;

	case XED_ICLASS_ADD:
		ADD(cpu);
		break;

	case XED_ICLASS_INC:
		INC(cpu);
		break;

	case XED_ICLASS_SUB:
		SUB(cpu);
		break;

	case XED_ICLASS_DEC:
		DEC(cpu);
		break;

	case XED_ICLASS_NEG_LOCK:
	case XED_ICLASS_NEG:
		NEG(cpu);
		break;

	case XED_ICLASS_CMP:
		CMP(cpu);
		break;

	case XED_ICLASS_MUL:
		MUL(cpu);
		break;

	case XED_ICLASS_IMUL:
		IMUL(cpu);
		break;

	case XED_ICLASS_DIV:
		DIV(cpu);
		break;

	case XED_ICLASS_IDIV:
		IDIV(cpu);
		break;

	case XED_ICLASS_XCHG:
		XCHG(cpu);
		break;

	case XED_ICLASS_NOT:
		NOT(cpu);
		break;

	case XED_ICLASS_IN:
		IN(cpu);
		break;

	case XED_ICLASS_OUT:
		OUT(cpu);
		break;

	case XED_ICLASS_XLAT:
		XLAT(cpu);
		break;

	case XED_ICLASS_LEA:
		LEA(cpu);
		break;

	case XED_ICLASS_LDS:
		LDS(cpu);
		break;

	case XED_ICLASS_LES:
		LES(cpu);
		break;

	case XED_ICLASS_LAHF:
		LAHF(cpu);
		break;

	case XED_ICLASS_SAHF:
		SAHF(cpu);
		break;

	case XED_ICLASS_PUSHF:
		PUSHF(cpu);
		break;

	case XED_ICLASS_POPF:
		POPF(cpu);
		break;

	case XED_ICLASS_CLC:
		CLC(cpu);
		break;

	case XED_ICLASS_STC:
		STC(cpu);
		break;

	case XED_ICLASS_CLI:
		CLI(cpu);
		break;

	case XED_ICLASS_STI:
		STI(cpu);
		break;

	case XED_ICLASS_CLD:
		CLD(cpu);
		break;

	case XED_ICLASS_STD:
		STD(cpu);
		break;

	case XED_ICLASS_HLT:
		HLT(cpu);
		break;
	
	case XED_ICLASS_NOP:
		NOP(cpu);
		break;
	
	default:
		hasExecutedInstruction = false;
	}

	cpu.ip += xed_decoded_inst_get_length(&cpu.eu.decodedInst);

	return hasExecutedInstruction;
}

static void execControlTransferInstruction()
{
	/* Control transfert */
	//TODO: Implement Jcc instructions
	switch (xed_decoded_inst_get_iclass(&cpu.eu.decodedInst))
	{
	case XED_ICLASS_CALL_NEAR:
		NEAR_CALL(cpu);
		break;

	case XED_ICLASS_CALL_FAR:
		FAR_CALL(cpu);
		break;

	case XED_ICLASS_JMP:
		NEAR_JMP(cpu);
		break;

	case XED_ICLASS_JMP_FAR:
		FAR_JMP(cpu);
		break;

	case XED_ICLASS_RET_NEAR:
		NEAR_RET(cpu);
		break;

	case XED_ICLASS_RET_FAR:
		FAR_RET(cpu);
		break;

	case XED_ICLASS_JZ:
		JZ(cpu);
		break;

	case XED_ICLASS_JL:
		JL(cpu);
		break;

	case XED_ICLASS_JLE:
		JLE(cpu);
		break;

	case XED_ICLASS_JNZ:
		JNZ(cpu);
		break;

	case XED_ICLASS_JNL:
		JNL(cpu);
		break;

	case XED_ICLASS_JNLE:
		JNLE(cpu);
		break;

	case XED_ICLASS_LOOP:
		LOOP(cpu);
		break;

	case XED_ICLASS_JCXZ:
		JCXZ(cpu);
		break;

	case XED_ICLASS_INT:
		INT(cpu);
		break;

	case XED_ICLASS_IRET:
		IRET(cpu);
		break;
	}
}

bool EU::clock()
{
	if (clockCountDown > 0)
	{
		clockCountDown -= 1;
		return false;
	}

	xed_decoded_inst_zero_keep_mode(&decodedInst);

	if (xed_decode(&decodedInst,cpu.biu.instructionBufferQueue.data(),cpu.biu.instructionBufferQueuePos) == xed_error_enum_t::XED_ERROR_NONE)
	{
		//TODO: implement this with the use of xed iform and lookub table and see if that can improve the speed
		instruction = xed_decoded_inst_inst(&decodedInst);
		cpu.biu.instructionBufferQueuePop(xed_decoded_inst_get_length(&cpu.eu.decodedInst));
		//TODO: implement all instruction timing

		if (!execNonControlTransferInstruction())
			execControlTransferInstruction();

		return true;
	}
	return false;
}