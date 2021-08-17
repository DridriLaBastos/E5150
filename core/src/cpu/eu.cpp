#include "eu.hpp"
#include "arch.hpp"

#include "instructions.hpp"

using namespace E5150::I8086;

EU::EU(): newFetchAddress(false) {}

static void printCurrentInstruction()
{
#if defined(SEE_CURRENT_INST) || defined(SEE_ALL)
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	if (!inst)
		return;
	//std::cout << std::hex << cpu.cs << ":" << cpu.ip << " (" << cpu.genAddress(cpu.cs,cpu.ip) << ")" << std::dec << ": ";
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
#endif
}

static unsigned int execInstructionAndGetClockCycles(void)
{
	switch (xed_decoded_inst_get_iclass(&cpu.eu.decodedInst))
	{
	case XED_ICLASS_MOV:
		MOV();
		return getMOVCycles();

	case XED_ICLASS_PUSH:
		PUSH();
		return getPUSHCycles();

	case XED_ICLASS_POP:
		POP();
		return getPOPCycles();

	case XED_ICLASS_ADD:
		ADD();
		return getADDCycles();

	case XED_ICLASS_INC:
		INC();
		return getINCCycles();

	case XED_ICLASS_SUB:
		SUB();
		return getSUBCycles();

	case XED_ICLASS_DEC:
		DEC();
		return getDECCycles();
	
	case XED_ICLASS_NEG:
		NEG();
		return getNEGCycles();

	case XED_ICLASS_CMP:
		CMP();
		return getCMPCycles();

	case XED_ICLASS_MUL:
		MUL();
		return getMULCycles();

	case XED_ICLASS_IMUL:
		IMUL();
		return getIMULCycles();

	case XED_ICLASS_DIV:
		DIV();
		return getDIVCycles();

	case XED_ICLASS_IDIV:
		IDIV();
		return getIDIVCycles();

	case XED_ICLASS_XCHG:
		XCHG();
		return getXCHGCycles();

	case XED_ICLASS_NOT:
		NOT();
		return getNOTCycles();

	case XED_ICLASS_IN:
	IN();
	return 	getINCycles();

	case XED_ICLASS_OUT:
		OUT();
		return getOUTCycles();

	case XED_ICLASS_XLAT:
		XLAT();
		return getXLATCycles();

	case XED_ICLASS_LEA:
		LEA();
		return getLEACycles();

	case XED_ICLASS_LDS:
		LDS();
		return getLDSCycles();

	case XED_ICLASS_LES:
		LES();
		return getLESCycles();

	case XED_ICLASS_LAHF:
		LAHF();
		return getLAHFCycles();

	case XED_ICLASS_SAHF:
		SAHF();
		return getSAHFCycles();

	case XED_ICLASS_PUSHF:
		PUSHF();
		return getPUSHFCycles();

	case XED_ICLASS_POPF:
		POPF();
		return getPOPFCycles();

	case XED_ICLASS_CLC:
		CLC();
		return getCLCCycles();

	case XED_ICLASS_STC:
		STC();
		return getSTCCycles();

	case XED_ICLASS_CLI:
		CLI();
		return getCLICycles();

	case XED_ICLASS_STI:
		STI();
		return getSTICycles();

	case XED_ICLASS_CLD:
		CLD();
		return getCLDCycles();

	case XED_ICLASS_STD:
		STD();
		return getSTDCycles();

	case XED_ICLASS_HLT:
		HLT();
		return getHLTCycles();
	
	case XED_ICLASS_NOP:
		NOP();
		return getNOPCycles();
	
	case XED_ICLASS_CALL_NEAR:
		CALL_NEAR();
		return getCALL_NEARCycles();

	case XED_ICLASS_CALL_FAR:
		CALL_FAR();
		return getCALL_FARCycles();

	case XED_ICLASS_JMP:
		JMP_NEAR();
		return getJMP_NEARCycles();

	case XED_ICLASS_JMP_FAR:
		JMP_FAR();
		return getJMP_FARCycles();

	case XED_ICLASS_RET_NEAR:
		RET_NEAR();
		return getRET_NEARCycles();

	case XED_ICLASS_RET_FAR:
		RET_FAR();
		return getRET_FARCycles();

	case XED_ICLASS_JZ:
		JZ();
		return 	getJZCycles();

	case XED_ICLASS_JL:
		JL();
		return 	getJLCycles();

	case XED_ICLASS_JLE:
		JLE();
		return getJLECycles();

	case XED_ICLASS_JNZ:
		JNZ();
		return getJNZCycles();

	case XED_ICLASS_JNL:
		JNL();
		return getJNLCycles();

	case XED_ICLASS_JNLE:
		JNLE();
		return getJNLECycles();

	case XED_ICLASS_LOOP:
		LOOP();
		return getLOOPCycles();

	case XED_ICLASS_JCXZ:
		JCXZ();
		return getJCXZCycles();

	case XED_ICLASS_INT:
		INT();
		return getINTCycles();

	case XED_ICLASS_IRET:
		IRET();
		return getIRETCycles();
	
	default:
		return 0;
	}
}

void EU::push (const uint16_t data)
{ cpu.sp -= 2; cpu.biu.EURequestWriteWord(cpu.genAddress(cpu.ss,cpu.sp), data); }

uint16_t EU::pop (void)
{
	const uint16_t ret = cpu.biu.EURequestReadWord(cpu.genAddress(cpu.ss,cpu.sp)); cpu.sp += 2;
	return ret;
}

void EU::farCall (const uint16_t seg, const uint16_t offset)
{
	push(cpu.cs);
	push(cpu.ip);

	newCS = seg;
	newIP = offset;
	newFetchAddress = true;
}

void EU::farRet (void)
{
	newIP = pop();
	newCS = pop();
	newFetchAddress = true;
}

bool EU::clock()
{
	static unsigned int clockCountDown = 0;
	if (clockCountDown > 0)
	{
		printCurrentInstruction();
		//printf(" clock left: %d\n",clockCountDown);
		clockCountDown -= 1;
		return false;
	}
	
	/*bool ret = false;
	if (instructionExec != nullptr)
	{
		instructionExec();
		instructionExec = nullptr;
		ret = true;
		cpu.instructionExecuted += 1;
	}*/

	if (newFetchAddress)
	{
		cpu.biu.requestNewFetchAddress(newCS,newIP);
		cpu.biu.resetInstructionBufferQueue();
		newFetchAddress = false;
	}

	xed_decoded_inst_zero_keep_mode(&decodedInst);
	const xed_error_enum_t DECODE_STATUS = xed_decode(&decodedInst,cpu.biu.instructionBufferQueue.data(),cpu.biu.instructionBufferQueuePos);

	if (DECODE_STATUS == xed_error_enum_t::XED_ERROR_NONE)
	{
		//Does the happen at the begining or the end of the execution of the instruction ?
		cpu.biu.instructionBufferQueuePop(xed_decoded_inst_get_length(&cpu.eu.decodedInst));
		printCurrentInstruction(); //printf("\n");
		clockCountDown = execInstructionAndGetClockCycles();
		cpu.instructionExecuted += 1;
	}

	return true;
}

/**
 * This function compute the effective address for a memory operand and add the corresponding number of 
 * instructions cycles. The instruction cycles are picked up from https://zsmith.co/intel.php#ea :
 * 1.  disp: mod = 0b00 and rm = 0b110								+6
 * 2.  (BX,BP,SI,DI): mod = 0b00 and rm != 0b110 and rm = 0b1xx		+5
 * 3.  disp + (BX,BP,SI,DI): mod = 0b10 and rm = 0b1xx				+9
 * 4.1 (BP+DI, bx+SI): mod = 0b00 and rm = 0b01x					+7
 * 4.2 (BP+SI, bx+DI): mod = 0b00 and rm = 0b00x					+8
 * 5.1 disp + (BP+DI, bx+SI) +-> same as precedet with mod = 0b10	+11
 * 5.2 disp + (BP+SI, bx+DI) +										+12
 *
 * word operands at odd addresses	+4
 * segment override					+2
 */
unsigned int EU::getEAComputationClockCount()
{
	const unsigned int modrm = xed_decoded_inst_get_modrm(&decodedInst);
	const unsigned int mod = (modrm & 0b11000000) >> 6;
	const unsigned int rm = modrm & 0b111;

	unsigned int clockNeeded = 0;

	if (mod == 0b00)
	{
		//1. disp: mod == 0b00 and rm 0b110
		if (rm == 0b110)
			clockNeeded += 6;
		else
		{
			//2. (base,index) mod = 0b00 and rm = 0b1xx and rm != 0b110
			if (rm & 0b100)
				clockNeeded += 5;
			//4.1/4.2 base + index mod = 0b00 and rm = 0b01x/0b00x
			else
				clockNeeded += (rm & 0b10) ? 7 : 8;
		}
	}
	//mod = 0b10
	else
	{
		//3. disp + (base,index): mod = 0b10 rm = 0b1xx
		if (rm & 0b100)
			clockNeeded += 9;
		//5.1/5.2 base + index + disp: mod = 0b10 rm = 0b01x/0b00x
		else
			clockNeeded += (rm & 0b10) ? 11 : 12;
	}

	cpu.eu.EAAddress = cpu.genAddress(xed_decoded_inst_get_seg_reg(&decodedInst,0), xed_decoded_inst_get_memory_displacement(&decodedInst,0));
	
	if (xed_operand_values_has_segment_prefix(&decodedInst))
		clockNeeded += 2;
	
	return clockNeeded;
}
