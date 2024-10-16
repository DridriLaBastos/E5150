//
//  instructionsClockCycles.cpp
//  core
//
//  Created by Adrien COURNAND on 01/08/2021.
//

#include "core/arch.hpp"
#include "core/instructions.hpp"

#define CLOCK_CYCLES(...) static const uint8_t CLOCK_CYCLES [] = { __VA_ARGS__ }
#define GET_RAW_CLOCK_COUNT() unsigned int clockCount = CLOCK_CYCLES[xed_decoded_inst_get_iform_enum_dispatch(&E5150::Arch::cpu.decodedInst)];
//TODO: Implement the commented code
#define ADD_EA_ON_CONDITION(COND) if (COND) { clockCount += 0; /*ComputeEAComputationClockCount();*/ }

#define GET_IFORM() const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&E5150::Arch::cpu.decodedInst)
#define ADD_EA_ON_IFORM_CONDITION(COND) GET_RAW_CLOCK_COUNT();\
										GET_IFORM();\
										ADD_EA_ON_CONDITION(COND)

#define ADD_EA_ON_MEM_OPERAND() GET_RAW_CLOCK_COUNT();\
								if (xed_decoded_inst_number_of_memory_operands(&E5150::Arch::cpu.decodedInst) > 0){\
                                const xed_inst_t* inst = xed_decoded_inst_inst(&E5150::Arch::cpu.decodedInst);\
								const xed_operand_enum_t op0 = xed_operand_name(xed_inst_operand(inst, 0));\
								const xed_operand_enum_t op1 = xed_operand_name(xed_inst_operand(inst, 1));\
								ADD_EA_ON_CONDITION (op0 == XED_OPERAND_MEM0 || op1 == XED_OPERAND_MEM0)}

static unsigned int ComputeEAComputationClockCount()
{
	E5150::Intel8088& cpu = E5150::Arch::cpu;
	const unsigned int modrm = xed_decoded_inst_get_modrm(&cpu.decodedInst);
	const unsigned int mod = (modrm & 0b11000000) >> 6;
	const unsigned int rm = modrm & 0b111;

	unsigned int clockNeeded = 0;

	if (mod == 0b00)
	{
		//1. regs.diregs.sp: mod == 0b00 and rm 0b110
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
		//3. regs.diregs.sp + (base,index): mod = 0b10 rm = 0b1xx
		if (rm & 0b100)
			clockNeeded += 9;
			//5.1/5.2 base + index + regs.diregs.sp: mod = 0b10 rm = 0b01x/0b00x
		else
			clockNeeded += (rm & 0b10) ? 11 : 12;
	}

	if (xed_operand_values_has_segment_prefix(&cpu.decodedInst))
		clockNeeded += 2;

	return clockNeeded;
}

/**
	All the timings for the instructions are taken from this site : http://aturing.umcs.maine.edu/~meadow/courses/cos335/80x86-Integer-Instruction-Set-Clocks.pdf
	When operands include memory operations, +1 is added to the clock count because on the IBM 5150 all memory operation tool 5 clock cycles instead of 4
	The informations on xed iform format are available here : https://intelxed.github.io/ref-manual/group__IFORM.html

	"v" = 16, 32 or 64b
	"z" = 16 or 32b
 */

/* *** Data transfert *** */

unsigned int getMOVCycles()
{
	CLOCK_CYCLES(
		10,//XED_IFORM_MOV_AL_MEMb
		2,//XED_IFORM_MOV_GPR8_GPR8_88
		2,//XED_IFORM_MOV_GPR8_GPR8_8A
		4,//XED_IFORM_MOV_GPR8_IMMb_B0
		4,//XED_IFORM_MOV_GPR8_IMMb_C6r0
		8,//XED_IFORM_MOV_GPR8_MEMb
		2,//XED_IFORM_MOV_GPRv_GPRv_89
		2,//XED_IFORM_MOV_GPRv_GPRv_8B
		4,//XED_IFORM_MOV_GPRv_IMMv
		4,//XED_IFORM_MOV_GPRv_IMMz
		8,//XED_IFORM_MOV_GPRv_MEMv
		2,//XED_IFORM_MOV_GPRv_SEG
		10,//XED_IFORM_MOV_MEMb_AL
		9,//XED_IFORM_MOV_MEMb_GPR8
		10,//XED_IFORM_MOV_MEMb_IMMb
		9,//XED_IFORM_MOV_MEMv_GPRv
		10,//XED_IFORM_MOV_MEMv_IMMz
		10,//XED_IFORM_MOV_MEMv_OrAX
		9,//XED_IFORM_MOV_MEMw_SEG
		10,//XED_IFORM_MOV_OrAX_MEMv
		2,//XED_IFORM_MOV_SEG_GPR16
		8,//XED_IFORM_MOV_SEG_MEMw
	);

	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getPUSHCycles ()
{
	CLOCK_CYCLES(
		14,//XED_IFORM_PUSH_CS
		14,//XED_IFORM_PUSH_DS
		0,//XED_IFORM_PUSH_ES
		0,//XED_IFORM_PUSH_FS
		15,//XED_IFORM_PUSH_GPRv_50
		15,//XED_IFORM_PUSH_GPRv_FFr6
		0,//XED_IFORM_PUSH_GS
		0,//XED_IFORM_PUSH_IMMb
		0,//XED_IFORM_PUSH_IMMz
		17,//XED_IFORM_PUSH_MEMv
		14//XED_IFORM_PUSH_SS
	);
	
	ADD_EA_ON_IFORM_CONDITION(iform == XED_IFORM_PUSH_MEMv);
	return clockCount;
}

unsigned int getPOPCycles ()
{
	CLOCK_CYCLES(
		8,// XED_IFORM_POP_DS
		8,// XED_IFORM_POP_ES
		8,// XED_IFORM_POP_FS
		8,// XED_IFORM_POP_GPRv_58
		8,// XED_IFORM_POP_GPRv_8F
		8,// XED_IFORM_POP_GS
		18,// XED_IFORM_POP_MEMv
		8// XED_IFORM_POP_SS
	);
	
	ADD_EA_ON_IFORM_CONDITION(iform == XED_IFORM_POP_MEMv);
	return clockCount;
}

unsigned int getXCHGCycles ()
{
	CLOCK_CYCLES(
		4,// XED_IFORM_XCHG_GPR8_GPR8
		4,// XED_IFORM_XCHG_GPRv_GPRv
		3,// XED_IFORM_XCHG_GPRv_OrAX
		18,// XED_IFORM_XCHG_MEMb_GPR8
		18// XED_IFORM_XCHG_MEMv_GPRv
	);
	ADD_EA_ON_IFORM_CONDITION(iform == XED_IFORM_XCHG_MEMb_GPR8 || iform == XED_IFORM_XCHG_MEMv_GPRv);
	return clockCount;
}

unsigned int getINCycles ()
{
	CLOCK_CYCLES(
		12,// XED_IFORM_IN_AL_DX
		14,// XED_IFORM_IN_AL_IMMb
		12,// XED_IFORM_IN_OeAX_DX
		14 // XED_IFORM_IN_OeAX_IMMb
	);

	GET_RAW_CLOCK_COUNT();
	return clockCount;
}

unsigned int getOUTCycles ()
{
	CLOCK_CYCLES(
		12,// XED_IFORM_OUT_DX_AL
		12,// XED_IFORM_OUT_DX_OeAX
		14,// XED_IFORM_OUT_IMMb_AL
		14 // XED_IFORM_OUT_IMMb_OeAX
	);
	
	GET_RAW_CLOCK_COUNT();
	return clockCount;
}

unsigned int getXLATCycles () { return 11; }
unsigned int getLEACycles () { return 2 + ComputeEAComputationClockCount(); }
unsigned int getLDSCycles () { return 24 + ComputeEAComputationClockCount(); }
unsigned int getLESCycles () { return 24 + ComputeEAComputationClockCount(); }
unsigned int getLAHFCycles () { return 4; }
unsigned int getSAHFCycles () { return 4; }
unsigned int getPUSHFCycles () { return 14; }
unsigned int getPOPFCycles () { return 12; }

/* *** Data Transfert *** */

unsigned int getADDCycles()
{
	CLOCK_CYCLES(
		4,//XED_IFORM_ADD_AL_IMMb
		3,//XED_IFORM_ADD_GPR8_GPR8_00
		3,//XED_IFORM_ADD_GPR8_GPR8_02
		4,//XED_IFORM_ADD_GPR8_IMMb_80r0
		4,//XED_IFORM_ADD_GPR8_IMMb_82r0
		14,//XED_IFORM_ADD_GPR8_MEMb
		3,//XED_IFORM_ADD_GPRv_GPRv_01
		3,//XED_IFORM_ADD_GPRv_GPRv_03
		4,//XED_IFORM_ADD_GPRv_IMMb
		4,//XED_IFORM_ADD_GPRv_IMMz
		14,//XED_IFORM_ADD_GPRv_MEMv
		24,//XED_IFORM_ADD_MEMb_GPR8
		23,//XED_IFORM_ADD_MEMb_IMMb_80r0
		23,//XED_IFORM_ADD_MEMb_IMMb_82r0
		24,//XED_IFORM_ADD_MEMv_GPRv
		23,//XED_IFORM_ADD_MEMv_IMMb
		23,//XED_IFORM_ADD_MEMv_IMMz
		4//XED_IFORM_ADD_OrAX_IMMz
	);
	
	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getADCCycles	()
{
	CLOCK_CYCLES(
		4,//XED_IFORM_ADC_AL_IMMb
		3,//XED_IFORM_ADC_GPR8_GPR8_10
		3,//XED_IFORM_ADC_GPR8_GPR8_12
		4,//XED_IFORM_ADC_GPR8_IMMb_80r2
		4,//XED_IFORM_ADC_GPR8_IMMb_82r2
		14,//XED_IFORM_ADC_GPR8_MEMb
		3,//XED_IFORM_ADC_GPRv_GPRv_11
		3,//XED_IFORM_ADC_GPRv_GPRv_13
		4,//XED_IFORM_ADC_GPRv_IMMb
		4,//XED_IFORM_ADC_GPRv_IMMz
		14,//XED_IFORM_ADC_GPRv_MEMv
		24,//XED_IFORM_ADC_MEMb_GPR8
		23,//XED_IFORM_ADC_MEMb_IMMb_80r2
		23,//XED_IFORM_ADC_MEMb_IMMb_82r2
		24,//XED_IFORM_ADC_MEMv_GPRv
		23,//XED_IFORM_ADC_MEMv_IMMb
		23,//XED_IFORM_ADC_MEMv_IMMz
		4,//XED_IFORM_ADC_OrAX_IMMz
	);

	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getINCCycles ()
{
	CLOCK_CYCLES(
		3,// XED_IFORM_INC_GPR8
		3,// XED_IFORM_INC_GPRv_40
		3,// XED_IFORM_INC_GPRv_FFr0
		24,// XED_IFORM_INC_MEMb
		24// XED_IFORM_INC_MEMv
	);

	ADD_EA_ON_IFORM_CONDITION(iform >= XED_IFORM_INC_MEMb);
	return clockCount;
}

unsigned int getAAACycles	() { return 8; }
unsigned int getDAACycles	() { return 4; }

unsigned int getSUBCycles	()
{
	CLOCK_CYCLES(
		4,// XED_IFORM_SUB_AL_IMMb
		3,// XED_IFORM_SUB_GPR8_GPR8_28
		3,// XED_IFORM_SUB_GPR8_GPR8_2A
		4,// XED_IFORM_SUB_GPR8_IMMb_80r5
		4,// XED_IFORM_SUB_GPR8_IMMb_82r5
		14,// XED_IFORM_SUB_GPR8_MEMb
		3,// XED_IFORM_SUB_GPRv_GPRv_29
		3,// XED_IFORM_SUB_GPRv_GPRv_2B
		4,// XED_IFORM_SUB_GPRv_IMMb
		4,// XED_IFORM_SUB_GPRv_IMMz
		14,// XED_IFORM_SUB_GPRv_MEMv
		24,// XED_IFORM_SUB_MEMb_GPR8
		23,// XED_IFORM_SUB_MEMb_IMMb_80r5
		23,// XED_IFORM_SUB_MEMb_IMMb_82r5
		24,// XED_IFORM_SUB_MEMv_GPRv
		23,// XED_IFORM_SUB_MEMv_IMMb
		23,// XED_IFORM_SUB_MEMv_IMMz
		4// XED_IFORM_SUB_OrAX_IMMz
	);

	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getSBBCycles	()
{
	CLOCK_CYCLES(
		4,//XED_IFORM_SBB_AL_IMMb
		3,//XED_IFORM_SBB_GPR8_GPR8_18
		3,//XED_IFORM_SBB_GPR8_GPR8_1A
		4,//XED_IFORM_SBB_GPR8_IMMb_80r3
		4,//XED_IFORM_SBB_GPR8_IMMb_82r3
		14,//XED_IFORM_SBB_GPR8_MEMb
		3,//XED_IFORM_SBB_GPRv_GPRv_19
		3,//XED_IFORM_SBB_GPRv_GPRv_1B
		4,//XED_IFORM_SBB_GPRv_IMMb
		4,//XED_IFORM_SBB_GPRv_IMMz
		14,//XED_IFORM_SBB_GPRv_MEMv
		24,//XED_IFORM_SBB_MEMb_GPR8
		23,//XED_IFORM_SBB_MEMb_IMMb_80r3
		23,//XED_IFORM_SBB_MEMb_IMMb_82r3
		24,//XED_IFORM_SBB_MEMv_GPRv
		23,//XED_IFORM_SBB_MEMv_IMMb
		23,//XED_IFORM_SBB_MEMv_IMMz
		4//XED_IFORM_SBB_OrAX_IMMz
	);

	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getDECCycles	()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&E5150::Arch::cpu.decodedInst);
	const xed_operand_enum_t op0 = xed_operand_name(xed_inst_operand(inst, 0));

	return op0 == XED_OPERAND_MEM0 ? (23 + ComputeEAComputationClockCount()) : 3;
}

unsigned int getNEGCycles	()
{
	const xed_inst_t* inst = xed_decoded_inst_inst(&E5150::Arch::cpu.decodedInst);
	const xed_operand_enum_t op0 = xed_operand_name(xed_inst_operand(inst, 0));

	return op0 == XED_OPERAND_MEM0 ? (24 + ComputeEAComputationClockCount()) : 3;
}

unsigned int getCMPCycles	()
{
	CLOCK_CYCLES(
		4,// XED_IFORM_CMP_AL_IMMb
		3,// XED_IFORM_CMP_GPR8_GPR8_38
		3,// XED_IFORM_CMP_GPR8_GPR8_3A
		4,// XED_IFORM_CMP_GPR8_IMMb_80r7
		4,// XED_IFORM_CMP_GPR8_IMMb_82r7
		14,// XED_IFORM_CMP_GPR8_MEMb
		3,// XED_IFORM_CMP_GPRv_GPRv_39
		3,// XED_IFORM_CMP_GPRv_GPRv_3B
		4,// XED_IFORM_CMP_GPRv_IMMb
		4,// XED_IFORM_CMP_GPRv_IMMz
		14,// XED_IFORM_CMP_GPRv_MEMv
		24,// XED_IFORM_CMP_MEMb_GPR8
		23,// XED_IFORM_CMP_MEMb_IMMb_80r7
		23,// XED_IFORM_CMP_MEMb_IMMb_82r7
		24,// XED_IFORM_CMP_MEMv_GPRv
		23,// XED_IFORM_CMP_MEMv_IMMb
		23,// XED_IFORM_CMP_MEMv_IMMz
		4// XED_IFORM_CMP_OrAX_IMMz
	);

	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getAASCycles	() { return 8; }
unsigned int getDASCycles	() { return 4; }

//TODO: find what defines the clock count for different iform of the instruction, for now the average is taken
unsigned int getMULCycles	()
{
	CLOCK_CYCLES(
		(70 + 77) / 2,// XED_IFORM_MUL_GPR8
		(118 + 133) / 2,// XED_IFORM_MUL_GPRv
		(76 + 83) / 2,// XED_IFORM_MUL_MEMb
		(124 + 139) / 2// XED_IFORM_MUL_MEMv
	);

	ADD_EA_ON_IFORM_CONDITION(iform == XED_IFORM_MUL_MEMb || iform == XED_IFORM_MUL_MEMv);
	return clockCount;
}

//TODO: find what defines the clock count for different iform of the instruction, for now the average is taken
unsigned int getIMULCycles	()
{
	CLOCK_CYCLES(
		(80 + 98) / 2,// XED_IFORM_IMUL_GPR8
		(128 + 154) / 2,// XED_IFORM_IMUL_GPRv
		(86 + 104) / 2,// XED_IFORM_IMUL_MEMb
		(134 + 160) / 2// XED_IFORM_IMUL_MEMv
	);

	ADD_EA_ON_IFORM_CONDITION(iform == XED_IFORM_IMUL_MEMb || iform == XED_IFORM_IMUL_MEMv);
	return clockCount;
}

unsigned int getDIVCycles	()
{
	CLOCK_CYCLES(
		(80 + 90) / 2,// XED_IFORM_DIV_GPR8
		(144 + 162) / 2,// XED_IFORM_DIV_GPRv
		(86 + 96) / 2,// XED_IFORM_DIV_MEMb
		(150 + 168) / 2// XED_IFORM_DIV_MEMv
	);

	ADD_EA_ON_IFORM_CONDITION(iform == XED_IFORM_DIV_MEMb || iform == XED_IFORM_DIV_MEMv);
	return clockCount;
}

unsigned int getIDIVCycles	()
{
	CLOCK_CYCLES(
		(101 + 112) / 2,// XED_IFORM_IDIV_GPR8
		(165 + 184) / 2,// XED_IFORM_IDIV_GPRv
		(107 + 118) / 2,// XED_IFORM_IDIV_MEMb
		(171 + 190) / 2// XED_IFORM_IDIV_MEMv
	);

	ADD_EA_ON_IFORM_CONDITION(iform == XED_IFORM_IDIV_MEMb || iform == XED_IFORM_IDIV_MEMv);
	return clockCount;
}
unsigned int getAADCycles	() { return 60; }
unsigned int getCBWCycles	() { return 2; }
unsigned int getCWDCycles	() { return 5; }

/* Logic */

unsigned int getNOTCycles	()
{
	CLOCK_CYCLES(
		3,// XED_IFORM_NOT_GPR8
		3,// XED_IFORM_NOT_GPRv
		24,// XED_IFORM_NOT_MEMb
		24// XED_IFORM_NOT_MEMv
	);

	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

//Same clock values for shift and rotate
unsigned int getSHIFT_ROTATECycles (const unsigned int nPrefix)
{
	CLOCK_CYCLES(
		8,// XED_IFORM_SHL_GPR8_CL_D2r4
		8,// XED_IFORM_SHL_GPR8_CL_D2r6
		// XED_IFORM_SHL_GPR8_IMMb_C0r4 (not implemented in 808x)
		// XED_IFORM_SHL_GPR8_IMMb_C0r6 (not implemented in 808x)
		2,// XED_IFORM_SHL_GPR8_ONE_D0r4
		2,// XED_IFORM_SHL_GPR8_ONE_D0r6
		8,// XED_IFORM_SHL_GPRv_CL_D3r4
		8,// XED_IFORM_SHL_GPRv_CL_D3r6
		// XED_IFORM_SHL_GPRv_IMMb_C1r4 (not implemented in 808x)
		// XED_IFORM_SHL_GPRv_IMMb_C1r6 (not implemented in 808x)
		2,// XED_IFORM_SHL_GPRv_ONE_D1r4
		2,// XED_IFORM_SHL_GPRv_ONE_D1r6
		20,// XED_IFORM_SHL_MEMb_CL_D2r4
		20,// XED_IFORM_SHL_MEMb_CL_D2r6
		// XED_IFORM_SHL_MEMb_IMMb_C0r4 (not implemented in 808x)
		// XED_IFORM_SHL_MEMb_IMMb_C0r6 (not implemented in 808x)
		15,// XED_IFORM_SHL_MEMb_ONE_D0r4
		15,// XED_IFORM_SHL_MEMb_ONE_D0r6
		20,// XED_IFORM_SHL_MEMv_CL_D3r4
		20,// XED_IFORM_SHL_MEMv_CL_D3r6
		// XED_IFORM_SHL_MEMv_IMMb_C1r4 (not implemented in 808x)
		// XED_IFORM_SHL_MEMv_IMMb_C1r6 (not implemented in 808x)
		15,// XED_IFORM_SHL_MEMv_ONE_D1r4
		15// XED_IFORM_SHL_MEMv_ONE_D1r6
	);

	ADD_EA_ON_MEM_OPERAND();
	if (E5150::Arch::cpu.instructionStreamQueue[nPrefix] & 0b10)
		clockCount += 4 * E5150::Arch::cpu.regs.cl;
	
	return clockCount;
}
// unsigned int getSHLCycles	() { return 7 + 0; }
// unsigned int getSHRCycles	() { return 7 + 0; }
// unsigned int getSARCycles	() { return 7 + 0; }
// unsigned int getROLCycles	() { return 7 + 0; }
// unsigned int getRORCycles	() { return 7 + 0; }
// unsigned int getRCLCycles	() { return 7 + 0; }
// unsigned int getRCRCycles	() { return 7 + 0; }

unsigned int getANDCycles	()
{
	CLOCK_CYCLES(
		4,// XED_IFORM_AND_AL_IMMb
		3,// XED_IFORM_AND_GPR8_GPR8_20
		3,// XED_IFORM_AND_GPR8_GPR8_22
		4,// XED_IFORM_AND_GPR8_IMMb_80r4
		4,// XED_IFORM_AND_GPR8_IMMb_82r4
		14,// XED_IFORM_AND_GPR8_MEMb
		3,// XED_IFORM_AND_GPRv_GPRv_21
		3,// XED_IFORM_AND_GPRv_GPRv_23
		4,// XED_IFORM_AND_GPRv_IMMb
		4,// XED_IFORM_AND_GPRv_IMMz
		14,// XED_IFORM_AND_GPRv_MEMv
		24,// XED_IFORM_AND_MEMb_GPR8
		23,// XED_IFORM_AND_MEMb_IMMb_80r4
		23,// XED_IFORM_AND_MEMb_IMMb_82r4
		24,// XED_IFORM_AND_MEMv_GPRv
		23,// XED_IFORM_AND_MEMv_IMMb
		23,// XED_IFORM_AND_MEMv_IMMz
		4// XED_IFORM_AND_OrAX_IMMz
	);
	
	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}
unsigned int getTESTCycles	()
{
	CLOCK_CYCLES(
		4,// XED_IFORM_TEST_AL_IMMb
		3,// XED_IFORM_TEST_GPR8_GPR8
		5,// XED_IFORM_TEST_GPR8_IMMb_F6r0
		5,// XED_IFORM_TEST_GPR8_IMMb_F6r1
		3,// XED_IFORM_TEST_GPRv_GPRv
		5,// XED_IFORM_TEST_GPRv_IMMz_F7r0
		5,// XED_IFORM_TEST_GPRv_IMMz_F7r1
		13,// XED_IFORM_TEST_MEMb_GPR8
		11,// XED_IFORM_TEST_MEMb_IMMb_F6r0
		11,// XED_IFORM_TEST_MEMb_IMMb_F6r1
		13,// XED_IFORM_TEST_MEMv_GPRv
		11,// XED_IFORM_TEST_MEMv_IMMz_F7r0
		11,// XED_IFORM_TEST_MEMv_IMMz_F7r1
		4// XED_IFORM_TEST_OrAX_IMMz
	);

	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}
unsigned int getORCycles	()
{
	CLOCK_CYCLES(
		4,// XED_IFORM_OR_AL_IMMb
		3,// XED_IFORM_OR_GPR8_GPR8_08
		3,// XED_IFORM_OR_GPR8_GPR8_0A
		4,// XED_IFORM_OR_GPR8_IMMb_80r1
		4,// XED_IFORM_OR_GPR8_IMMb_82r1
		14,// XED_IFORM_OR_GPR8_MEMb
		3,// XED_IFORM_OR_GPRv_GPRv_09
		3,// XED_IFORM_OR_GPRv_GPRv_0B
		4,// XED_IFORM_OR_GPRv_IMMb
		4,// XED_IFORM_OR_GPRv_IMMz
		14,// XED_IFORM_OR_GPRv_MEMv
		24,// XED_IFORM_OR_MEMb_GPR8
		23,// XED_IFORM_OR_MEMb_IMMb_80r1
		23,// XED_IFORM_OR_MEMb_IMMb_82r1
		24,// XED_IFORM_OR_MEMv_GPRv
		23,// XED_IFORM_OR_MEMv_IMMb
		23,// XED_IFORM_OR_MEMv_IMMz
		4// XED_IFORM_OR_OrAX_IMMz
	);
	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getXORCycles	()
{
	CLOCK_CYCLES(
		4,// XED_IFORM_XOR_AL_IMMb
		3,// XED_IFORM_XOR_GPR8_GPR8_30
		3,// XED_IFORM_XOR_GPR8_GPR8_32
		4,// XED_IFORM_XOR_GPR8_IMMb_80r6
		4,// XED_IFORM_XOR_GPR8_IMMb_82r6
		14,// XED_IFORM_XOR_GPR8_MEMb
		3,// XED_IFORM_XOR_GPRv_GPRv_31
		3,// XED_IFORM_XOR_GPRv_GPRv_33
		4,// XED_IFORM_XOR_GPRv_IMMb
		4,// XED_IFORM_XOR_GPRv_IMMz
		14,// XED_IFORM_XOR_GPRv_MEMv
		24,// XED_IFORM_XOR_MEMb_GPR8
		23,// XED_IFORM_XOR_MEMb_IMMb_80r6
		23,// XED_IFORM_XOR_MEMb_IMMb_82r6
		24,// XED_IFORM_XOR_MEMv_GPRv
		23,// XED_IFORM_XOR_MEMv_IMMb
		23,// XED_IFORM_XOR_MEMv_IMMz
		4// XED_IFORM_XOR_OrAX_IMMz
	);
	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

/* String Manipulation */
#if 0
unsigned int getMOVSCycles () { return cpu.eu.operandSizeWord ? 26 : 18; }
unsigned int getREP_MOVSCycles (const unsigned int repeatCount) { return repeatCount == 0 ? 9 : (cpu.eu.operandSizeWord ? 25 : 17); }
//TODO: find accurate clock cycles
unsigned int getCMPSCycles () { return cpu.eu.operandSizeWord ? 30 : 22; }
unsigned int getREP_CMPSCycles (const unsigned int repeatCount) { return repeatCount == 0 ? 9 : 30; }
unsigned int getSCASCycles () { return 19; }
unsigned int getREP_SCASCycles (const unsigned int repeatCount) { return repeatCount == 0 ? 9 : (cpu.eu.operandSizeWord ? 19 : 15); }
unsigned int getLODSCycles () { return 16; }
//TODO: clock value from my mind, I didn't find infos about the rep clock cycle version
unsigned int getREP_LODSCycles (const unsigned int repeatCount) { return repeatCount == 0 ? 9 : (cpu.eu.operandSizeWord ? 15 : 14); }
unsigned int getSTOSCycles () { return cpu.eu.operandSizeWord ? 15 : 11; }
unsigned int getREP_STOSCycles (const unsigned int repeatCount) { return repeatCount == 0 ? 9 : (cpu.eu.operandSizeWord ? 14 : 10); }
#endif
/* Control Transfer */

// unsigned int getCALL_NEARCycles	(){}
// unsigned int getCALL_FARCycles	() { return 7 + 0; }
unsigned int getCALLCycles		()
{
	CLOCK_CYCLES(
		21,// XED_IFORM_CALL_NEAR_GPRv
		16,// XED_IFORM_CALL_NEAR_MEMv
		0,// XED_IFORM_CALL_NEAR_RELBRd (not implemented on 808x)
		19// XED_IFORM_CALL_NEAR_RELBRz
	);

	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getCALL_FARCycles	()
{
	CLOCK_CYCLES(
		 37,// XED_IFORM_CALL_FAR_MEMp2
		 28,// XED_IFORM_CALL_FAR_PTRp_IMMw
	);
	
	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

// unsigned int getJMP_NEARCycles	() { return 7 + 0; }
// unsigned int getJMP_FARCycles	() { return 7 + 0; }
//TODO: verify the value returned
unsigned int getJMPCycles		()
{
	CLOCK_CYCLES(
		11,// XED_IFORM_JMP_GPRv
		18,// XED_IFORM_JMP_MEMv
		15,// XED_IFORM_JMP_RELBRb
		0,// XED_IFORM_JMP_RELBRd (not implemented on 808x)
		15,// XED_IFORM_JMP_RELBRz
		24,// XED_IFORM_JMP_FAR_MEMp2
		24// XED_IFORM_JMP_FAR_PTRp_IMMw
	);

	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getJMP_FARCycles	()
{
	CLOCK_CYCLES(
		 15,//XED_IFORM_JMP_FAR_MEMp2
		 15,//XED_IFORM_JMP_FAR_PTRp_IMMw
	);
	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getRETCycles		()
{
	CLOCK_CYCLES(
		20,// XED_IFORM_RET_NEAR
		24// XED_IFORM_RET_NEAR_IMMw
	);

	GET_RAW_CLOCK_COUNT();
	return clockCount;
}

unsigned int getRET_FARCycles	()
{
	CLOCK_CYCLES(
		34,// XED_IFORM_RET_FAR
		33,// XED_IFORM_RET_FAR_IMMw
	);
	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getJXXCycles		(const bool conditionValue) { return conditionValue ? 16 : 4; }
// unsigned int getJZCycles		() { return 7 + 0; }/* JE /JZ   */
// unsigned int getJLCycles		() { return 7 + 0; }/* JL /JNGE */
// unsigned int getJLECycles		() { return 7 + 0; }/* JLE/JNG  */
// unsigned int getJBCycles		() { return 7 + 0; }/* JB /JNAE */
// unsigned int getJBECycles		() { return 7 + 0; }/* JBE/JNA  */
// unsigned int getJPCycles		() { return 7 + 0; }/* JLE/JNG  */
// unsigned int getJOCycles		() { return 7 + 0; }/* JP /JPE  */
// unsigned int getJSCycles		() { return 7 + 0; }
// unsigned int getJNZCycles		() { return 7 + 0; }
// unsigned int getJNLCycles		() { return 7 + 0; }
// unsigned int getJNLECycles		() { return 7 + 0; }
// unsigned int getJNBCycles		() { return 7 + 0; }
// unsigned int getJNBECycles		() { return 7 + 0; }
// unsigned int getJNPCycles		() { return 7 + 0; }
// unsigned int getJNSCycles		() { return 7 + 0; }
#if 0
unsigned int getLOOPCycles		() { return (E5150::Arch::cpu.regs.cx - 1 == 0) ? 5 : 18; }
unsigned int getLOOPZCycles		() { return ((E5150::Arch::cpu.regs.cx - 1 == 0) && cpu.getFlagStatus(CPU::ZERRO)) ? 6 : 18; }
unsigned int getLOOPNZCycles	() { return ((E5150::Arch::cpu.regs.cx - 1 == 0) && !cpu.getFlagStatus(CPU::ZERRO)) ? 5 : 19; }
#endif
unsigned int getJCXZCycles		() { return E5150::Arch::cpu.regs.cx == 0 ? 6 : 18; }
unsigned int getINTCycles		() { return 51; }
unsigned int getINT3Cycles		() { return 52; }
//Only return the value if the interrupt sequence needs to be performed. If not, this instruction will be executed as a normal instruction with clock cycle value of 4
unsigned int getINTOCycles		() { return 53; }
unsigned int getINTRCycles		() { return 61; }
unsigned int getNMICycles		() { return 50; }

unsigned int getIRETCycles		() { return 44; }

/* Processor Control */

unsigned int getCLCCycles	() { return 2; }
unsigned int getCMCCycles	() { return 2; }
unsigned int getSTCCycles	() { return 2; }
unsigned int getCLDCycles	() { return 2; }
unsigned int getSTDCycles	() { return 2; }
unsigned int getCLICycles	() { return 2; }
unsigned int getSTICycles	() { return 2; }
unsigned int getHLTCycles	() { return 2; }
unsigned int getWAITCycles	() { return 7 + 0; }
unsigned int getLOCKCycles	() { return 7 + 0; }

unsigned int getNOPCycles	() { return 3; }
