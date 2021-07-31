#include "eu.hpp"
#include "arch.hpp"

#include "instructions.hpp"

using namespace E5150::I8086;

EU::EU(): mClockCountDown(0) {}

static void printCurrentInstruction()
{
#if defined(SEE_CURRENT_INST) || defined(SEE_ALL)
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.decodedInst);
	if (!inst)
		return;
	std::cout << std::hex << cpu.cs << ":" << cpu.ip << " (" << cpu.genAddress(cpu.cs,cpu.ip) << ")" << std::dec << ": ";
	//std::cout << xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(&cpu.decodedInst)) << " : length = " << xed_decoded_inst_get_length(&cpu.decodedInst) << std::endl;
	std::cout << xed_iclass_enum_t2str(xed_decoded_inst_get_iclass(&cpu.decodedInst)) << " ";
	unsigned int realOperandPos = 0;
	bool foundPtr = false;

	for (unsigned int i = 0; i < xed_decoded_inst_noperands(&cpu.decodedInst); ++i)
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
				std::cout << xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
				break;

			case XED_OPERAND_PTR:
				std::cout << xed_decoded_inst_get_branch_displacement(&cpu.decodedInst);
				foundPtr = true;
				break;

			case XED_OPERAND_REG0:
			case XED_OPERAND_REG1:
			case XED_OPERAND_REG2:
				std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_reg(&cpu.decodedInst, op_name));
				break;

			case XED_OPERAND_IMM0:
			case XED_OPERAND_IMM1:
				std::cout << std::hex << xed_decoded_inst_get_unsigned_immediate(&cpu.decodedInst) << std::dec;
				break;

			//Displaying memory operand woth format SEG:[[BASE +] [INDEX +] DISPLACEMENT ]
			case XED_OPERAND_MEM0:
			{
				const xed_reg_enum_t baseReg = xed_decoded_inst_get_base_reg(&cpu.decodedInst, 0);
				const xed_reg_enum_t indexReg = xed_decoded_inst_get_index_reg(&cpu.decodedInst, 0);
				const int64_t memDisplacement = xed_decoded_inst_get_memory_displacement(&cpu.decodedInst,0);
				std::cout << ((xed_decoded_inst_get_memory_operand_length(&cpu.decodedInst, 0) == 1) ? "BYTE" : "WORD") << " ";
				std::cout << xed_reg_enum_t2str(xed_decoded_inst_get_seg_reg(&cpu.decodedInst, 0)) << ":[";

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



void EU::clock()
{
	if (mClockCountDown > 0)
	{
		mClockCountDown -= 1;
		return;
	}

	xed_decoded_inst_zero_keep_mode(&decodedInst);

	if (xed_decode(&decodedInst,cpu.biu.instructionBufferQueue.data(),cpu.biu.instructionBufferQueuePos) == xed_error_enum_t::XED_ERROR_NONE)
	{
		printCurrentInstruction();
		instruction = xed_decoded_inst_inst(&decodedInst);
		cpu.biu.instructionBufferQueuePop(xed_decoded_inst_get_length(&cpu.decodedInst));
		mClockCountDown = 7;
	}
	else
		printf("INVALID!\n");
}