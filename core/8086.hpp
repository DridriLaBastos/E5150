#ifndef __I8086_HPP__
#define __I8086_HPP__

#if 0
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
		//static functions defined in the header files so they can be inlined by the compiler
		static void ConfigureXed(xed_decoded_inst_t* decodedInst)
		{
			xed_decoded_inst_zero(decodedInst);
			xed_decoded_inst_set_mode(decodedInst, XED_MACHINE_MODE_REAL_16, XED_ADDRESS_WIDTH_16b);
		}

		static unsigned int genAddress (const uint16_t base, const uint16_t offset)
		{
			return (base << 4) + offset;
		}

	public:
		Regs regs;

		E5150::I8086::BIU biu;
		E5150::I8086::EU eu;

		uint64_t instructionExecutedCount;
};
#endif

namespace E5150
{
	class Intel8088
	{
	public:
		static constexpr unsigned int MEMORY_FETCH_CLOCK_COUNT = 5;
		static constexpr unsigned int INSTRUCTION_STREAM_QUEUE_LENGTH = 5;
		static constexpr unsigned int INSTRUCTION_STREAM_QUEUE_INDEX_MAX = INSTRUCTION_STREAM_QUEUE_LENGTH - 1;
	public:
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

		enum class ERunningMode
		{
			INIT_SEQUENCE, OPERATIONAL
		};

		enum class EBIURunningMode
		{
			FETCH_MEMORY,WAIT_ROOM_IN_QUEUE
		};

		enum class EBIUFetchType
		{
			FETCH_INSTRUCTION, FETCH_DATA
		};

		enum class EEURunningMode
		{
			WAIT_INSTRUCTION,EXECUTE_INSTRUCTION,EXECUTE_REP_INSTRUCTION,WAIT_BIU
		};

		enum class EEventFlags
		{
			INSTRUCTION_EXECUTED = 1 << 0,
			INSTRUCTION_DECODED  = 1 << 1,
		};

		enum class ECpuFlags
		{
			CARRY	= 1 <<  0, // carry flag
			PARITY	= 1 <<  2, // parity flag
			A_CARRY	= 1 <<  4, // auxiliary carry flag
			ZERO	= 1 <<  6, // zerro flag
			SIGN	= 1 <<  7, // sign flag
			TRAP	= 1 <<  8, // trap flag
			INTF	= 1 <<  9, // interrupt flag
			DIR		= 1 << 10, // direction flag
			OVER	= 1 << 11  // overflow flag
		};

	public:
		Intel8088(void);

		void Clock(void);

		template <typename... Flags>
		void ClearFlags(const Flags& ...flags)
		{
			regs.flags &= ~HelperGetFlagMask(flags...);
		}

		template <typename... Flags>
		void SetFlags(const Flags... flags)
		{
			regs.flags |= HelperGetFlagMask(flags...);
		}

		template <typename... Flags>
		void ToggleFlags(const Flags... flags)
		{
			regs.flags ^= HelperGetFlagMask(flags...);
		}

	private:
		template<typename Flag, typename... OtherFlags>
		unsigned int HelperGetFlagMask(const Flag flag, const OtherFlags... otherFlags)
		{
			return static_cast<unsigned int>(flag) | HelperGetFlagMask(otherFlags...);
		}

		unsigned int HelperGetFlagMask(const ECpuFlags flag)
		{
			return static_cast<unsigned int>(flag);
		}

	public:
		Regs regs;
		ERunningMode mode;
		EEURunningMode euMode;
		EBIURunningMode biuMode;
		EBIUFetchType biuCurrentFetchType,biuNextFetchType;

		xed_decoded_inst_t decodedInst;
		xed_iclass_enum_t  decodedInstructionIClass;

		uint8_t instructionStreamQueue [INSTRUCTION_STREAM_QUEUE_LENGTH];
		size_t instructionStreamQueueIndex;

		unsigned int events;

		unsigned int biuClockCountDown;
		unsigned int biuByteRequest;
		
		unsigned int euClockCountDown;
		unsigned int euInstructionClockCount;

		unsigned int clock;

		bool halted;
	};
}

#endif
