#ifndef __DECODER_HELPER_HPP__
#define __DECODER_HELPER_HPP__

/**
 * This header contains data that are usefull to the decoder but that I don't want to be in decoder.hpp to not poluate this header and keep it nice and simple
 */

namespace E5150::I8086::DECODER
{
	enum INSTRUCTION_PARTIAL_NAME
	{
		MOV_DW, MOV_W_REG, MOV_W1, MOV_W2,
		PUSH_REG, PUSH_SEG,
		POP_REG, POP_SEG,
		XCHG_W, XCHG_REG,
		IN_W1, IN_W2, OUT_W1, OUT_W2,
		XLAT, LEA, LDS, LES, LAHF, SAHF, PUSHF, POPF,

		INCOMPLETE
	};

	enum class INSTRUCTION_NAME
	{ MOV, PUSH, POP, XCHG, IN, OUT, XLAT, LEA, LDS, LES, LAHF, SAHF, PUSHF, POPF };

	enum class INSTRUCTION_PREFIX
	{ REP, NONE };

	enum class OPERAND
	{ REGISTER, MEMORY, IMMEDIATE };

	///enum containing all the registers that can be found in an instruction
	enum class REGISTER_NAME
	{ AX, BX, CX, DX, BP, SP, DI, SI, AL, BL, CL, DL, AH, BH, CH, DH, CS, DS, ES, SS };

	/**
	 * \brief Enumeration of the different steps of the decoding process.
	 * 
	 * PTR is used for long pointer operand
	 * \enum 
	 */
	enum class INSTRUCTION_STEP
	{ FIRST_BYTE, INCOMPLETE_NEXT, MOD_REG_RM_BYTE, DATA, ADDR, DISPLACEMENT, PTR };

	/**
	 * \brief Enumeration of all the form of the available instructions.
	 * 
	 * This enumeration is not exhaustive, for each instruction only form that are inherantly different or with different clock cycles for similar forms are listed. For exemple if an instruction takes 10 cyckes to do an operation between two general purpose registers, and the same amount to do the same operation between a general purpose register and a segment register, the only listed form for this instruction will be *_REG_REG.
	 * But, if an instruction takes 10 cycles to complete for with two registers, and 10 cyckes to complete for one register and a memory address, the two form will still be listed
	 */
	enum class INSTRUCTION_FORM
	{
		MOV_MEM_ACC,//stands for acc to mem and mem to acc since the both take 10 cycles
		MOV_REG_REG,
		MOV_REG_MEM,
		MOV_MEM_REG,
		MOV_REG_IMM,
		MOV_MEM_IMM,

		PUSH_REG,
		PUSH_SEG,
		PUSH_MEM,

		POP_REG,
		POP_SEG,
		POP_MEM,

		XCHG_ACC_REG,
		XCHG_MEM_REG,
		XCHG_REG_REG
	};
}

#endif