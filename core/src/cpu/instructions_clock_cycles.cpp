//
//  instructionsClockCycles.cpp
//  core
//
//  Created by Adrien COURNAND on 01/08/2021.
//

#include "arch.hpp"
#include "instructions.hpp"

///Return clock without effective addresse calculation time
#define GET_RAW_CLOCK_COUNT() unsigned int clockCount = CLOCK_CYCLES[xed_decoded_inst_get_iform_enum_dispatch(&cpu.eu.decodedInst)]
#define ADD_EA_COMPUTATION_ON_CONDITION(COND) if (COND) { clockCount += cpu.eu.getEAComputationClockCount(); }
#define GET_IFORM() const xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(&cpu.eu.decodedInst)
#define ADD_EA_COMPUTATION_ON_IFORM_CONDITION(COND) GET_RAW_CLOCK_COUNT();\
													GET_IFORM();\
													ADD_EA_COMPUTATION_ON_CONDITION(COND)

/**
	All th timings for the instructions are taken from this site : https://zsmith.co/intel.php
	The informations on xed iform format are available here : https://intelxed.github.io/ref-manual/group__IFORM.html
 */

/* *** Data transfert *** */

unsigned int getMOVCycles()
{
	static const uint8_t CLOCK_CYCLES [] =
	{
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
		10,//XED_IFORM_MOV_MEMv_IMMz (not present on 808x)
		10,//XED_IFORM_MOV_MEMv_OrAX
		9,//XED_IFORM_MOV_MEMw_SEG
		10,//XED_IFORM_MOV_OrAX_MEMv
		2,//XED_IFORM_MOV_SEG_GPR16
		8,//XED_IFORM_MOV_SEG_MEMw
	};

	GET_RAW_CLOCK_COUNT();
	GET_IFORM();

	if (iform == XED_IFORM_MOV_AL_MEMb || iform == XED_IFORM_MOV_MEMb_AL ||
		iform == XED_IFORM_MOV_OrAX_MEMv || iform == XED_IFORM_MOV_MEMv_OrAX)
		return clockCount;
	
	const xed_inst_t* inst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op0 = xed_operand_name(xed_inst_operand(inst, 0));
	const xed_operand_enum_t op1 = xed_operand_name(xed_inst_operand(inst, 1));
	
	ADD_EA_COMPUTATION_ON_CONDITION(op0 == XED_OPERAND_MEM0 || op1 == XED_OPERAND_MEM0);
	return clockCount;
}

unsigned int getPUSHCycles (void)
{
	static const uint8_t CLOCK_CYCLES [] =
	{
		14,//XED_IFORM_PUSH_CS
		14,//XED_IFORM_PUSH_DS
		0,//XED_IFORM_PUSH_ES
		0,//XED_IFORM_PUSH_FS
		15,//XED_IFORM_PUSH_GPRv_50
		15,//XED_IFORM_PUSH_GPRv_FFr6
		0,//XED_IFORM_PUSH_GS
		0,//XED_IFORM_PUSH_IMMb
		0,//XED_IFORM_PUSH_IMMz
		16,//XED_IFORM_PUSH_MEMv
		14//XED_IFORM_PUSH_SS
	};
	
	ADD_EA_COMPUTATION_ON_IFORM_CONDITION(iform == XED_IFORM_PUSH_MEMv);
	return clockCount;
}

unsigned int getPOPCycles (void)
{
	static const uint8_t CLOCK_CYCLES [] =
	{
		8,// XED_IFORM_POP_DS
		8,// XED_IFORM_POP_ES
		8,// XED_IFORM_POP_FS
		8,// XED_IFORM_POP_GPRv_58
		8,// XED_IFORM_POP_GPRv_8F
		8,// XED_IFORM_POP_GS
		17,// XED_IFORM_POP_MEMv
		8// XED_IFORM_POP_SS
	};
	
	ADD_EA_COMPUTATION_ON_IFORM_CONDITION(iform == XED_IFORM_POP_MEMv);
	return clockCount;
}

unsigned int getXCHGCycles (void)
{
	static const uint8_t CLOCK_CYCLES [] =
	{
		4,// XED_IFORM_XCHG_GPR8_GPR8
		4,// XED_IFORM_XCHG_GPRv_GPRv
		3,// XED_IFORM_XCHG_GPRv_OrAX
		17,// XED_IFORM_XCHG_MEMb_GPR8
		17// XED_IFORM_XCHG_MEMv_GPRv
	};
	ADD_EA_COMPUTATION_ON_IFORM_CONDITION(iform == XED_IFORM_XCHG_MEMb_GPR8 || iform == XED_IFORM_XCHG_MEMv_GPRv);
	return clockCount;
}

unsigned int getINCycles (void) { return 7; }
unsigned int getOUTCycles (void) { return 7; }
unsigned int getXLATCycles (void) { return 7; }
unsigned int getLEACycles (void) { return 7; }
unsigned int getLDSCycles (void) { return 7; }
unsigned int getLESCycles (void) { return 7; }
unsigned int getLAHFCycles (void) { return 7; }
unsigned int getSAHFCycles (void) { return 7; }
unsigned int getPUSHFCycles (void) { return 7; }
unsigned int getPOPFCycles (void) { return 7; }

/* *** Data Transfert *** */

unsigned int getADDCycles()
{
	static const uint8_t CLOCK_CYCLES [] =
	{
		4,//XED_IFORM_ADD_AL_IMMb
		3,//XED_IFORM_ADD_GPR8_GPR8_00
		3,//XED_IFORM_ADD_GPR8_GPR8_02
		4,//XED_IFORM_ADD_GPR8_IMMb_80r0
		4,//XED_IFORM_ADD_GPR8_IMMb_82r0
		9,//XED_IFORM_ADD_GPR8_MEMb
		3,//XED_IFORM_ADD_GPRv_GPRv_01
		3,//XED_IFORM_ADD_GPRv_GPRv_03
		4,//XED_IFORM_ADD_GPRv_IMMb
		4,//XED_IFORM_ADD_GPRv_IMMz (not present on 808x)
		9,//XED_IFORM_ADD_GPRv_MEMv
		16,//XED_IFORM_ADD_MEMb_GPR8
		17,//XED_IFORM_ADD_MEMb_IMMb_80r0
		17,//XED_IFORM_ADD_MEMb_IMMb_82r0
		16,//XED_IFORM_ADD_MEMv_GPRv
		17//XED_IFORM_ADD_MEMv_IMMb
	};
	
	const unsigned int clockCount = CLOCK_CYCLES[xed_decoded_inst_get_iform_enum_dispatch(&cpu.eu.decodedInst)];
	cpu.eu.xedInst = xed_decoded_inst_inst(&cpu.eu.decodedInst);
	const xed_operand_enum_t op0 = xed_operand_name(xed_inst_operand(cpu.eu.xedInst, 0));
	const xed_operand_enum_t op1 = xed_operand_name(xed_inst_operand(cpu.eu.xedInst, 1));
	
	return clockCount + ((op0 == XED_OPERAND_MEM0 || op1 == XED_OPERAND_MEM0) ? cpu.eu.getEAComputationClockCount() : 0);
}

unsigned int getINCCycles (void) { return 7; }
unsigned int getSUBCycles (void) { return 7; }
unsigned int getDECCycles (void) { return 7; }
unsigned int getNEGCycles (void) { return 7; }
unsigned int getCMPCycles (void) { return 7; }
unsigned int getMULCycles (void) { return 7; }
unsigned int getIMULCycles (void) { return 7; }
unsigned int getDIVCycles (void) { return 7; }
unsigned int getIDIVCycles (void) { return 7; }

/* *** Control Transfert *** */

unsigned int getCALL_NEARCycles (void) { return 32; }
unsigned int getCALL_FARCycles (void) { return 32; }
unsigned int getJMP_NEARCycles (void) { return 32; }
unsigned int getJMP_FARCycles (void) { return 32; }
unsigned int getRET_NEARCycles (void) { return 32; }
unsigned int getRET_FARCycles (void) { return 32; }
unsigned int getJZCycles (void) { return 32; }
unsigned int getJLCycles (void) { return 32; }
unsigned int getJLECycles (void) { return 32; }
unsigned int getJNZCycles (void) { return 32; }
unsigned int getJNLCycles (void) { return 32; }
unsigned int getJNLECycles (void) { return 32; }
unsigned int getLOOPCycles (void) { return 32; }
unsigned int getJCXZCycles (void) { return 32; }
unsigned int getINTCycles (void) { return 32; }
unsigned int getIRETCycles (void) { return 32; }

unsigned int getNOTCycles (void) { return 2; }

/* *** CPU Control *** */
unsigned int getCLCCycles (void) { return 3; }
unsigned int getSTCCycles (void) { return 3; }
unsigned int getCLDCycles (void) { return 3; }
unsigned int getSTDCycles (void) { return 3; }
unsigned int getCLICycles (void) { return 3; }
unsigned int getSTICycles (void) { return 3; }
unsigned int getHLTCycles (void) { return 3; }
unsigned int getNOPCycles (void) { return 3; }
