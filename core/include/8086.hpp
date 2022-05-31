#ifndef __I8086_HPP__
#define __I8086_HPP__

#include "eu.hpp"
#include "biu.hpp"
#include "util.hpp"

union reg_t
{
	//Represents both the 'x' value ax,bx,cx,dx registers, and the integer value (the 'v') of other registers
	union
	{
		uint16_t x;
		uint16_t v;
	};
	struct
	{
		uint8_t l;
		uint8_t h;
	};

	reg_t (const uint16_t v = 0): x(v) {}
	
	void operator= (const uint16_t v) { x = v; }
	operator uint16_t (void) const { return x; }
	void operator+= (const uint16_t v) { x += v; }
	void operator-= (const uint16_t v) { x -= v; }
	void operator*= (const uint16_t v) { x *= v; }
	void operator/= (const uint16_t v) { x /= v; }
	void operator%= (const uint16_t v) { x %= v; }

	void operator&= (const uint16_t v) { x &= v; }
	void operator|= (const uint16_t v) { x |= v; }
	void operator^= (const uint16_t v) { x ^= v; }

	uint16_t operator++ (void) { return x++; }
	uint16_t operator++ (const int unused) { return ++x; }
	uint16_t operator-- (void) { return x--; }
	uint16_t operator-- (const int unused) { return --x; }
	
};

//TODO: go throught all the instructions and implements 
// - exceptions
// - prefixes
//TODO: Add a namespace here
class CPU
{
	public:
		CPU(void);

		unsigned int clock (void);
		/**
		 * @brief Perform the interrupt sequence
		 * 
		 * @return true The interrupt sequence needs to be relaunch because IF or TF was on
		 * @return false The interrupt sequence finishes without a need to relaunch
		 */
		bool interruptSequence(void);
	
	public:
		enum REGISTERS
		{ AX, BX, CX, DX,
		  SI, DI, BP, SP,
		  CS, DS, ES, SS, 
		  IP, FLAGS, NUM
		};

		enum FLAGS_T
		{
			CARRY	= 1 <<  0, // carry flag
			PARRITY	= 1 <<  2, // parity flag
			A_CARRY	= 1 <<  4, // auxiliary carry flag
			ZERRO	= 1 <<  6, // zerro flag
			SIGN	= 1 <<  7, // sign flag
			TRAP	= 1 <<  8, // trap flag
			INTF	= 1 <<  9, // interrupt flag
			DIR		= 1 << 10, // direction flag
			OVER	= 1 << 11  // overflow flag
		};

		enum class INTERRUPT_TYPE
		{ NMI, INT3, INTO, INTERNAL, EXTERNAL, DIVIDE };
	
	/* *** UTILITY FUNCTIONS NEEDED BY INSTRUCTIONS AND OTHER COMPONENTS *** */
	public:
		void interrupt(const CPU::INTERRUPT_TYPE type, const uint8_t vector=0);
		void handleInterrupts (void);
		void iretDelay(void);

		unsigned int genAddress (const uint16_t base, const uint16_t offset) const;
		unsigned int genAddress (const uint16_t base, const xed_reg_enum_t offset) const;
		unsigned int genAddress (const xed_reg_enum_t segment, const uint16_t offset) const;
		unsigned int genAddress (const xed_reg_enum_t segment, const xed_reg_enum_t offset) const;
		unsigned int genEA (void);

		void push (const uint16_t data);
		uint16_t pop (void);

		void testCF (const unsigned int value, const bool byte);
		void testPF	(const unsigned int value);
		void testAF	(const unsigned int value);
		void testZF	(const unsigned int value);
		void testSF	(const unsigned int value);
		void testOF	(const unsigned int value, const bool byte);
		void setFlags	(const unsigned int flags);
		void toggleFlags (const unsigned int flags);
		void clearFlags	(const unsigned int flags);
		void updateFlag (const FLAGS_T& flag, const bool value);

		bool getFlagStatus	(const FLAGS_T flag) const;

		void updateStatusFlags (const unsigned int value, const bool byte);

		uint16_t readReg  (const xed_reg_enum_t reg) const;
		void write_reg  (const xed_reg_enum_t reg, const unsigned int data);
		void hlt(void);

	public:
		xed_decoded_inst_t decodedInst;
		std::array<reg_t, REGISTERS::NUM> regs;

		#define ax regs[CPU::REGISTERS::AX].x
		#define al regs[CPU::REGISTERS::AX].l
		#define ah regs[CPU::REGISTERS::AX].h

		#define bx regs[CPU::REGISTERS::BX].x
		#define bl regs[CPU::REGISTERS::BX].l
		#define bh regs[CPU::REGISTERS::BX].h

		#define cx regs[CPU::REGISTERS::CX].x
		#define cl regs[CPU::REGISTERS::CX].l
		#define ch regs[CPU::REGISTERS::CX].h

		#define dx regs[CPU::REGISTERS::DX].x
		#define dl regs[CPU::REGISTERS::DX].l
		#define dh regs[CPU::REGISTERS::DX].h

		#define si regs[CPU::REGISTERS::SI].v
		#define di regs[CPU::REGISTERS::DI].v
		#define bp regs[CPU::REGISTERS::BP].v
		#define sp regs[CPU::REGISTERS::SP].v

		#define ds regs[CPU::REGISTERS::DS].v
		#define cs regs[CPU::REGISTERS::CS].v
		#define es regs[CPU::REGISTERS::ES].v
		#define fs regs[CPU::REGISTERS::FS].v
		#define ss regs[CPU::REGISTERS::SS].v

		#define ip regs[CPU::REGISTERS::IP].v
		#define flags regs[CPU::REGISTERS::FLAGS].v

		E5150::I8086::BIU biu;
		E5150::I8086::EU eu;

		uint64_t instructionExecutedCount;
};

#endif
