#ifndef __I8086_HPP__
#define __I8086_HPP__

#include "eu.hpp"
#include "biu.hpp"
#include "util.hpp"

struct Regs {
	union {
		uint16_t ax;
		struct {
			uint16_t al : 8, ah : 8;
		};
	};

	union {
		uint16_t bx;
		struct {
			uint16_t bl : 8, bh : 8;
		};
	};

	union {
		uint16_t cx;
		struct {
			uint16_t cl : 8, ch : 8;
		};
	};

	union {
		uint16_t dx;
		struct {
			uint16_t dl : 8, dh : 8;
		};
	};

	uint16_t cs, ds, es, ss, si, di, bp, sp, ip, flags;
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
		Regs regs;

		E5150::I8086::BIU biu;
		E5150::I8086::EU eu;

		uint64_t instructionExecutedCount;
};

#endif
