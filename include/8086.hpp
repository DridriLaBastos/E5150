#ifndef __I8086_HPP__
#define __I8086_HPP__

#include "util.hpp"

#include "ram.hpp"
#include "ports.hpp"

using reg_t = uint16_t;

union greg_t
{
	uint16_t x;
	struct { uint16_t l:8, h:8; };
};

namespace E5150{ class Arch; }

//TODO: IMPORTANT: Implement Max Mode
//TODO: go throught all the instructions and implements 
// - exceptions
// - prefixes
class CPU
{
	public:
		CPU(RAM& ram, PORTS& ports);

		void simulate (void);
		void request_nmi (void);
		void request_intr (const uint8_t vector);
		
	private:
		unsigned int gen_address (const reg_t base, const uint16_t offset) const;
		unsigned int gen_address (const reg_t base, const xed_reg_enum_t offset) const;
		unsigned int gen_address (const xed_reg_enum_t segment, const uint16_t offset) const;
		unsigned int gen_address (const xed_reg_enum_t segment, const xed_reg_enum_t offset) const;
		unsigned int genEA (const xed_operand_enum_t op_name);

		uint8_t  readByte (const unsigned int addr) const;
		uint16_t readWord (const unsigned int addr) const;

		void writeByte (const unsigned int addr, const uint8_t  data);
		void writeWord (const unsigned int addr, const uint16_t data);

		void testCF (const unsigned int value, const bool byte);
		void testPF	(const unsigned int value);
		void testAF	(const unsigned int value);
		void testZF	(const unsigned int value);
		void testSF	(const unsigned int value);
		void testOF	(const unsigned int value, const bool byte);
		void setFlags	(const unsigned int flags);
		void clearFlags	(const unsigned int flags);
		bool getFlagStatus	(const unsigned int flag);

		void updateStatusFlags (const unsigned int value, const bool byte);

		void write_reg  (const xed_reg_enum_t reg, const unsigned int data);
		uint16_t read_reg  (const xed_reg_enum_t reg) const;

		uint16_t pop (void);
		void push (const uint16_t data);
		void far_call (const reg_t seg, const uint16_t offset);
		void far_ret (void);
		void interrupt (void);
	
		void printRegisters	(void) const;
		void printFlags		(void) const;
		void printCurrentInstruction (void) const;

		bool execNonControlTransferInstruction (void);
		void execControlTransferInstruction (void);

	private:
		enum GREG
		{ AX, BX, CX, DX };

		enum REG
		{
			SI, DI, BP, SP,
			CS, DS, ES, SS
		};

		enum FLAGS
		{
			CARRY	= 1 <<  0, // carry flag
			PARRITY	= 1 <<  2, // parity flag
			A_CARRY	= 1 <<  4, // auxiliary carry flag
			ZERRO	= 1 <<  6, // zerro flag
			SIGN	= 1 <<  7, // sign flag
			TRAP	= 1 <<  8, // trap flag
			INTF	= 1 <<  9, // interrupt flag
			DIR		= 1 << 10, // direction flag
			OVER	= 1 << 11, // overflow flag
		};

	private:
		//TODO: continue to implement data transfer instructions
		/* Data Transfer */
		void MOV	(void);
		void PUSH	(void);
		void POP	(void);
		void XCHG	(void);
		void IN		(void);
		void OUT	(void);
		void XLAT	(void);
		void LEA	(void);
		void LDS	(void);
		void LES	(void);
		void LAHF	(void);
		void SAHF	(void);
		void PUSHF	(void);
		void POPF	(void);

		//TODO: continue to implement arithmetic instructions
		/* Arithmetic */
		void ADD  (void);
		void INC  (void);
		void SUB  (void);
		void DEC  (void);
		void NEG  (void);
		void CMP  (void);
		void MUL  (void);
		void IMUL (void);
		void DIV  (void);
		void IDIV (void);

		/* Control Transfer */
		void NEAR_CALL	(void);
		void FAR_CALL	(void);
		void NEAR_JMP	(void);
		void FAR_JMP	(void);
		void NEAR_RET	(void);
		void FAR_RET	(void);
		void JZ			(void);/*  JZ/JE   */
		void JL			(void);/*  JL/JNGE */
		void JLE		(void);/* JLE/JNG  */
		void JNZ		(void);
		void JNL		(void);
		void JNLE		(void);
		void LOOP		(void);
		void JCXZ		(void);
		void INT		(void);
		void IRET		(void);

		void NOT (void);

		//TODO: continue to implements processor control instructions
		/* Processor Control */
		void CLC (void);
		void STC (void);
		void CLD (void);
		void STD (void);
		void CLI (void);
		void STI (void);
		void HLT (void);

	private:
		std::array<greg_t, 4> m_gregs;
		std::array<reg_t, 8> m_regs;
		reg_t m_flags;
		reg_t m_ip;
		
	private:
		bool hlt;
		bool nmi;
		bool intr;
		bool interrupt_enable;
		uint8_t intr_v;
		unsigned int fault_count;
		unsigned int m_clockCountDown;
		
	private:
		RAM&	m_ram;
		PORTS&	m_ports;
		xed_decoded_inst_t m_decoded_inst;
};

#endif
