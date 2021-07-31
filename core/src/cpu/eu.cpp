#include "eu.hpp"
#include "arch.hpp"

#include "instructions.hpp"

using namespace E5150::I8086;

EU::EU(): clockCountDown(0), instructionGetClockCount(nullptr), instructionExec(nullptr) {}

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

static void fillInstructionsFunctionPtr(void)
{
	switch (xed_decoded_inst_get_iclass(&cpu.eu.decodedInst))
	{
	case XED_ICLASS_MOV:
		cpu.eu.instructionGetClockCount = getMOVCycles;
		cpu.eu.instructionExec			= MOV;
		break;

	case XED_ICLASS_PUSH:
		cpu.eu.instructionGetClockCount = getPUSHCycles;
		cpu.eu.instructionExec			= PUSH;
		break;

	case XED_ICLASS_POP:
		cpu.eu.instructionGetClockCount = getPOPCycles;
		cpu.eu.instructionExec			= POP;
		break;

	case XED_ICLASS_ADD:
		cpu.eu.instructionGetClockCount = getADDCycles;
		cpu.eu.instructionExec			= ADD;
		break;

	case XED_ICLASS_INC:
		cpu.eu.instructionGetClockCount = getINCCycles;
		cpu.eu.instructionExec			= INC;
		break;

	case XED_ICLASS_SUB:
		cpu.eu.instructionGetClockCount = getSUBCycles;
		cpu.eu.instructionExec			= SUB;
		break;

	case XED_ICLASS_DEC:
		cpu.eu.instructionGetClockCount = getDECCycles;
		cpu.eu.instructionExec			= DEC;
		break;

	/*case XED_ICLASS_NEG_LOCK:
		cpu.eu.instructionGetClockCount = getNEG_LOCKCycles;
		cpu.eu.instructionExec			= NEG_LOCK*/
	
	case XED_ICLASS_NEG:
		cpu.eu.instructionGetClockCount = getNEGCycles;
		cpu.eu.instructionExec			= NEG;
		break;

	case XED_ICLASS_CMP:
		cpu.eu.instructionGetClockCount = getCMPCycles;
		cpu.eu.instructionExec			= CMP;
		break;

	case XED_ICLASS_MUL:
		cpu.eu.instructionGetClockCount = getMULCycles;
		cpu.eu.instructionExec			= MUL;
		break;

	case XED_ICLASS_IMUL:
		cpu.eu.instructionGetClockCount = getIMULCycles;
		cpu.eu.instructionExec			= IMUL;
		break;

	case XED_ICLASS_DIV:
		cpu.eu.instructionGetClockCount = getDIVCycles;
		cpu.eu.instructionExec			= DIV;
		break;

	case XED_ICLASS_IDIV:
		cpu.eu.instructionGetClockCount = getIDIVCycles;
		cpu.eu.instructionExec			= IDIV;
		break;

	case XED_ICLASS_XCHG:
		cpu.eu.instructionGetClockCount = getXCHGCycles;
		cpu.eu.instructionExec			= XCHG;
		break;

	case XED_ICLASS_NOT:
		cpu.eu.instructionGetClockCount = getNOTCycles;
		cpu.eu.instructionExec			= NOT;
		break;

	case XED_ICLASS_IN:
		cpu.eu.instructionGetClockCount = getINCycles;
		cpu.eu.instructionExec			= IN;
		break;

	case XED_ICLASS_OUT:
		cpu.eu.instructionGetClockCount = getOUTCycles;
		cpu.eu.instructionExec			= OUT;
		break;

	case XED_ICLASS_XLAT:
		cpu.eu.instructionGetClockCount = getXLATCycles;
		cpu.eu.instructionExec			= XLAT;
		break;

	case XED_ICLASS_LEA:
		cpu.eu.instructionGetClockCount = getLEACycles;
		cpu.eu.instructionExec			= LEA;
		break;

	case XED_ICLASS_LDS:
		cpu.eu.instructionGetClockCount = getLDSCycles;
		cpu.eu.instructionExec			= LDS;
		break;

	case XED_ICLASS_LES:
		cpu.eu.instructionGetClockCount = getLESCycles;
		cpu.eu.instructionExec			= LES;
		break;

	case XED_ICLASS_LAHF:
		cpu.eu.instructionGetClockCount = getLAHFCycles;
		cpu.eu.instructionExec			= LAHF;
		break;

	case XED_ICLASS_SAHF:
		cpu.eu.instructionGetClockCount = getSAHFCycles;
		cpu.eu.instructionExec			= SAHF;
		break;

	case XED_ICLASS_PUSHF:
		cpu.eu.instructionGetClockCount = getPUSHFCycles;
		cpu.eu.instructionExec			= PUSHF;
		break;

	case XED_ICLASS_POPF:
		cpu.eu.instructionGetClockCount = getPOPFCycles;
		cpu.eu.instructionExec			= POPF;
		break;

	case XED_ICLASS_CLC:
		cpu.eu.instructionGetClockCount = getCLCCycles;
		cpu.eu.instructionExec			= CLC;
		break;

	case XED_ICLASS_STC:
		cpu.eu.instructionGetClockCount = getSTCCycles;
		cpu.eu.instructionExec			= STC;
		break;

	case XED_ICLASS_CLI:
		cpu.eu.instructionGetClockCount = getCLICycles;
		cpu.eu.instructionExec			= CLI;
		break;

	case XED_ICLASS_STI:
		cpu.eu.instructionGetClockCount = getSTICycles;
		cpu.eu.instructionExec			= STI;
		break;

	case XED_ICLASS_CLD:
		cpu.eu.instructionGetClockCount = getCLDCycles;
		cpu.eu.instructionExec			= CLD;
		break;

	case XED_ICLASS_STD:
		cpu.eu.instructionGetClockCount = getSTDCycles;
		cpu.eu.instructionExec			= STD;
		break;

	case XED_ICLASS_HLT:
		cpu.eu.instructionGetClockCount = getHLTCycles;
		cpu.eu.instructionExec			= HLT;
		break;
	
	case XED_ICLASS_NOP:
		cpu.eu.instructionGetClockCount = getNOPCycles;
		cpu.eu.instructionExec			= NOP;
		break;
	
	case XED_ICLASS_CALL_NEAR:
		cpu.eu.instructionGetClockCount = getCALL_NEARCycles;
		cpu.eu.instructionExec			= CALL_NEAR;
		break;

	case XED_ICLASS_CALL_FAR:
		cpu.eu.instructionGetClockCount = getCALL_FARCycles;
		cpu.eu.instructionExec			= CALL_FAR;
		break;

	case XED_ICLASS_JMP:
		cpu.eu.instructionGetClockCount = getJMP_NEARCycles;
		cpu.eu.instructionExec			= JMP_NEAR;
		break;

	case XED_ICLASS_JMP_FAR:
		cpu.eu.instructionGetClockCount = getJMP_FARCycles;
		cpu.eu.instructionExec			= JMP_FAR;
		break;

	case XED_ICLASS_RET_NEAR:
		cpu.eu.instructionGetClockCount = getRET_NEARCycles;
		cpu.eu.instructionExec			= RET_NEAR;
		break;

	case XED_ICLASS_RET_FAR:
		cpu.eu.instructionGetClockCount = getRET_FARCycles;
		cpu.eu.instructionExec			= RET_FAR;
		break;

	case XED_ICLASS_JZ:
		cpu.eu.instructionGetClockCount = getJZCycles;
		cpu.eu.instructionExec			= JZ;
		break;

	case XED_ICLASS_JL:
		cpu.eu.instructionGetClockCount = getJLCycles;
		cpu.eu.instructionExec			= JL;
		break;

	case XED_ICLASS_JLE:
		cpu.eu.instructionGetClockCount = getJLECycles;
		cpu.eu.instructionExec			= JLE;
		break;

	case XED_ICLASS_JNZ:
		cpu.eu.instructionGetClockCount = getJNZCycles;
		cpu.eu.instructionExec			= JNZ;
		break;

	case XED_ICLASS_JNL:
		cpu.eu.instructionGetClockCount = getJNLCycles;
		cpu.eu.instructionExec			= JNL;
		break;

	case XED_ICLASS_JNLE:
		cpu.eu.instructionGetClockCount = getJNLECycles;
		cpu.eu.instructionExec			= JNLE;
		break;

	case XED_ICLASS_LOOP:
		cpu.eu.instructionGetClockCount = getLOOPCycles;
		cpu.eu.instructionExec			= LOOP;
		break;

	case XED_ICLASS_JCXZ:
		cpu.eu.instructionGetClockCount = getJCXZCycles;
		cpu.eu.instructionExec			= JCXZ;
		break;

	case XED_ICLASS_INT:
		cpu.eu.instructionGetClockCount = getINTCycles;
		cpu.eu.instructionExec			= INT;
		break;

	case XED_ICLASS_IRET:
		cpu.eu.instructionGetClockCount = getIRETCycles;
		cpu.eu.instructionExec			= IRET;
		break;
	
	default:
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
	
	bool ret = false;
	if (instructionExec != nullptr)
	{
		instructionExec();
		instructionExec = nullptr;
		ret = true;
	}

	xed_decoded_inst_zero_keep_mode(&decodedInst);
	const xed_error_enum_t DECODE_STATUS = xed_decode(&decodedInst,cpu.biu.instructionBufferQueue.data(),cpu.biu.instructionBufferQueuePos);

	if (DECODE_STATUS == xed_error_enum_t::XED_ERROR_NONE)
	{
		//TODO: implement this with the use of xed iform and lookub table and see if that can improve the speed
		//instruction = xed_decoded_inst_inst(&decodedInst);
		cpu.biu.instructionBufferQueuePop(xed_decoded_inst_get_length(&cpu.eu.decodedInst));

		fillInstructionsFunctionPtr();
		clockCountDown = instructionGetClockCount();
	}

	return ret;
}

unsigned int EU::getEAComputationClockCount()
{
	
}