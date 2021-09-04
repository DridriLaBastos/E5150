//
//  instructionsClockCycles.cpp
//  core
//
//  Created by Adrien COURNAND on 01/08/2021.
//

#include "arch.hpp"
#include "instructions.hpp"

#define CLOCK_CYCLES(...) static const uint8_t CLOCK_CYCLES [] = { __VA_ARGS__ }
#define GET_RAW_CLOCK_COUNT() unsigned int clockCount = CLOCK_CYCLES[xed_decoded_inst_get_iform_enum_dispatch(&cpu.eu.decodedInst)]
#define ADD_EA_ON_CONDITION(COND) if (COND) { clockCount += cpu.eu.getEAComputationClockCount(); }

#define GET_IFORM() const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&cpu.eu.decodedInst)
#define ADD_EA_ON_IFORM_CONDITION(COND) GET_RAW_CLOCK_COUNT();\
										GET_IFORM();\
										ADD_EA_ON_CONDITION(COND)

#define ADD_EA_ON_MEM_OPERAND() GET_RAW_CLOCK_COUNT();\
								if (xed_decoded_inst_number_of_memory_operands(&cpu.eu.decodedInst) > 0){\
								cpu.eu.xedInst = xed_decoded_inst_inst(&cpu.eu.decodedInst);\
								const xed_operand_enum_t op0 = xed_operand_name(xed_inst_operand(cpu.eu.xedInst, 0));\
								const xed_operand_enum_t op1 = xed_operand_name(xed_inst_operand(cpu.eu.xedInst, 1));\
								ADD_EA_ON_CONDITION (op0 == XED_OPERAND_MEM0 || op1 == XED_OPERAND_MEM0)}

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
		11,//XED_IFORM_MOV_AL_MEMb
		2,//XED_IFORM_MOV_GPR8_GPR8_88
		2,//XED_IFORM_MOV_GPR8_GPR8_8A
		4,//XED_IFORM_MOV_GPR8_IMMb_B0
		4,//XED_IFORM_MOV_GPR8_IMMb_C6r0
		9,//XED_IFORM_MOV_GPR8_MEMb
		2,//XED_IFORM_MOV_GPRv_GPRv_89
		2,//XED_IFORM_MOV_GPRv_GPRv_8B
		4,//XED_IFORM_MOV_GPRv_IMMv
		4,//XED_IFORM_MOV_GPRv_IMMz
		9,//XED_IFORM_MOV_GPRv_MEMv
		3,//XED_IFORM_MOV_GPRv_SEG
		11,//XED_IFORM_MOV_MEMb_AL
		10,//XED_IFORM_MOV_MEMb_GPR8
		11,//XED_IFORM_MOV_MEMb_IMMb
		10,//XED_IFORM_MOV_MEMv_GPRv
		11,//XED_IFORM_MOV_MEMv_IMMz
		11,//XED_IFORM_MOV_MEMv_OrAX
		10,//XED_IFORM_MOV_MEMw_SEG
		11,//XED_IFORM_MOV_OrAX_MEMv
		2,//XED_IFORM_MOV_SEG_GPR16
		9,//XED_IFORM_MOV_SEG_MEMw
	);

	ADD_EA_ON_MEM_OPERAND();
	return clockCount;
}

unsigned int getPUSHCycles (void)
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

unsigned int getPOPCycles (void)
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

unsigned int getXCHGCycles (void)
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

unsigned int getINCycles (void)
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

unsigned int getOUTCycles (void)
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

unsigned int getXLATCycles (void) { return 11; }
unsigned int getLEACycles (void) { return 2 + cpu.eu.getEAComputationClockCount(); }
unsigned int getLDSCycles (void) { return 24 + cpu.eu.getEAComputationClockCount(); }
unsigned int getLESCycles (void) { return 24 + cpu.eu.getEAComputationClockCount(); }
unsigned int getLAHFCycles (void) { return 4; }
unsigned int getSAHFCycles (void) { return 4; }
unsigned int getPUSHFCycles (void) { return 14; }
unsigned int getPOPFCycles (void) { return 12; }

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

unsigned int getADCCycles	(void)
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

unsigned int getINCCycles (void)
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

unsigned int getAAACycles	(void) { return 8; }
unsigned int getDAACycles	(void) { return 4; }

unsigned int getSUBCycles	(void)
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

unsigned int getSBBCycles	(void)
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

unsigned int getDECCycles	(void)
{
	cpu.eu.xedInst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op0 = xed_operand_name(xed_inst_operand(cpu.eu.xedInst, 0));

	return op0 == XED_OPERAND_MEM0 ? (23 + cpu.eu.getEAComputationClockCount()) : 3;
}

unsigned int getNEGCycles	(void)
{
	cpu.eu.xedInst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op0 = xed_operand_name(xed_inst_operand(cpu.eu.xedInst, 0));

	return op0 == XED_OPERAND_MEM0 ? (24 + cpu.eu.getEAComputationClockCount()) : 3;
}

unsigned int getCMPCycles	(void)
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

unsigned int getAASCycles	(void) { return 8; }
unsigned int getDASCycles	(void) { return 4; }

//TODO: find what defines the clock count for different iform of the instruction, for now the average is taken
unsigned int getMULCycles	(void)
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
unsigned int getIMULCycles	(void)
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

unsigned int getDIVCycles	(void)
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

unsigned int getIDIVCycles	(void)
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
unsigned int getAADCycles	(void) { return 60; }
unsigned int getCBWCycles	(void) { return 2; }
unsigned int getCWDCycles	(void) { return 5; }

/* Logic */

unsigned int getNOTCycles	(void)
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
		8,// GPR8_CL
		0,// GPR8_IMMb (doesn't exist on 808x)
		2,// GPR8_ONE
		8,// GPRv_CL
		0,// GPRv_IMMb (doesn't exist on 808x)
		2,// GPRv_ONE
		28,// MEMb_CL
		0,// MEMb_IMMb (doesn't exist on 808x)
		23,// MEMb_ONE
		28,// MEMv_CL
		0,// MEMv_IMMb (doesn't exist on 808x)
		23// MEMv_ONE
	);

	ADD_EA_ON_MEM_OPERAND();
	if (cpu.biu.instructionBufferQueue[nPrefix] & 0b10)
		clockCount += 4 * cpu.cl;
	
	return clockCount;
}
// unsigned int getSHLCycles	(void) { return 7 + 0; }
// unsigned int getSHRCycles	(void) { return 7 + 0; }
// unsigned int getSARCycles	(void) { return 7 + 0; }
// unsigned int getROLCycles	(void) { return 7 + 0; }
// unsigned int getRORCycles	(void) { return 7 + 0; }
// unsigned int getRCLCycles	(void) { return 7 + 0; }
// unsigned int getRCRCycles	(void) { return 7 + 0; }

unsigned int getANDCycles	(void)
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
unsigned int getTESTCycles	(void)
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
unsigned int getORCycles	(void)
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

unsigned int getXORCycles	(void)
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

unsigned int getREPCycles (void) { return 7 + 0; }
unsigned int getMOVSCycles (void) { return 7 + 0; }
unsigned int getCMPSCycles (void) { return 7 + 0; }
unsigned int getSCASCycles (void) { return 7 + 0; }
unsigned int getLODSCycles (void) { return 7 + 0; }
unsigned int getSTOSCycles (void) { return 7 + 0; }

/* Control Transfer */

unsigned int getCALL_NEARCycles	(void) { return 7 + 0; }
unsigned int getCALL_FARCycles	(void) { return 7 + 0; }
unsigned int getJMP_NEARCycles	(void) { return 7 + 0; }
unsigned int getJMP_FARCycles	(void) { return 7 + 0; }
unsigned int getRET_NEARCycles	(void) { return 7 + 0; }
unsigned int getRET_FARCycles	(void) { return 7 + 0; }
unsigned int getJZCycles		(void) { return 7 + 0; }/* JE /JZ   */
unsigned int getJLCycles		(void) { return 7 + 0; }/* JL /JNGE */
unsigned int getJLECycles		(void) { return 7 + 0; }/* JLE/JNG  */
unsigned int getJBCycles		(void) { return 7 + 0; }/* JB /JNAE */
unsigned int getJBECycles		(void) { return 7 + 0; }/* JBE/JNA  */
unsigned int getJPCycles		(void) { return 7 + 0; }/* JLE/JNG  */
unsigned int getJOCycles		(void) { return 7 + 0; }/* JP /JPE  */
unsigned int getJSCycles		(void) { return 7 + 0; }
unsigned int getJNZCycles		(void) { return 7 + 0; }
unsigned int getJNLCycles		(void) { return 7 + 0; }
unsigned int getJNLECycles		(void) { return 7 + 0; }
unsigned int getJNBCycles		(void) { return 7 + 0; }
unsigned int getJNBECycles		(void) { return 7 + 0; }
unsigned int getJNPCycles		(void) { return 7 + 0; }
unsigned int getJNSCycles		(void) { return 7 + 0; }
unsigned int getLOOPCycles		(void) { return 7 + 0; }
unsigned int getLOOPZCycles		(void) { return 7 + 0; }
unsigned int getLOOPNZCycles	(void) { return 7 + 0; }
unsigned int getJCXZCycles		(void) { return 7 + 0; }
unsigned int getINTCycles		(void) { return 7 + 0; }
unsigned int getINTOCycles		(void) { return 7 + 0; }
unsigned int getIRETCycles		(void) { return 7 + 0; }

/* Processor Control */

unsigned int getCLCCycles	(void) { return 7 + 0; }
unsigned int getCMCCycles	(void) { return 7 + 0; }
unsigned int getSTCCycles	(void) { return 7 + 0; }
unsigned int getCLDCycles	(void) { return 7 + 0; }
unsigned int getSTDCycles	(void) { return 7 + 0; }
unsigned int getCLICycles	(void) { return 7 + 0; }
unsigned int getSTICycles	(void) { return 7 + 0; }
unsigned int getHLTCycles	(void) { return 7 + 0; }
unsigned int getWAITCycles	(void) { return 7 + 0; }
unsigned int getLOCKCycles	(void) { return 7 + 0; }

unsigned int getNOPCycles	(void) { return 7 + 0; }
