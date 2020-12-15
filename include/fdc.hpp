#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "pic.hpp"
#include "util.hpp"
#include "ports.hpp"
#include "floppy.hpp"
#include "component.hpp"

#define FDCDebug(REQUIRED_DEBUG_LEVEL,...) debug<REQUIRED_DEBUG_LEVEL>("FDC: " __VA_ARGS__)

namespace E5150
{
	/**
	 * This class will emulate the floppy disc controller in the IBM and the behaviour of the DOR register
	 */
	struct FDC: public Component
	{
		public:
			FDC(PIC& pic,PORTS& ports);

			void waitClock (const unsigned int clock)
			{ passClock += clock; debug<DEBUG_LEVEL_MAX>("FDC will wait {} clock(s)",passClock); }
			void waitMicro (const unsigned int microseconds) { waitClock(microseconds*8); }
			void waitMilli (const unsigned int milliseconds) { waitMicro(milliseconds*1000); }

			void interruptPIC(void) const;

			void switchToCommandMode (void);
			void switchToExecutionMode (void);
			void switchToResultMode (void);
			void makeBusy (void) { statusRegister |= (1 << 4); }
			void makeAvailable (void) { statusRegister &= ~(1 << 4); }
			void setSeekStatusOn (const unsigned int driveNumber) { statusRegister |= (1 << driveNumber); }
			void resetSeekStatusOf (const unsigned int driveNumber) { statusRegister &= ~(1 << driveNumber); }

			void makeDataRegisterReady (void);
			void makeDataRegisterNotReady (void);
			void makeDataRegisterInReadMode (void);
			void makeDataRegisterInWriteMode (void);
			
			bool isBusy (void) const { return statusRegister & 0b10000; }
			bool  dataRegisterReady (void) const;
		
		public:
			void clock (void);
		
			enum class PHASE
			{ COMMAND, EXECUTION, RESULT };

			enum DOR_REGISTER
			{
				DS0 = 1 << 0,
				DS1 = 1 << 1,
				FDC_RESET = 1 << 2,
				IO = 1 << 3,
				MA_ENABLE = 1 << 4,
				MB_ENABLE = 1 << 5,
				MC_ENABLE = 1 << 6,
				MD_ENABLE = 1 << 7
			};

			enum ST0_FLAGS
			{
				US0 = 1 << 0,
				US1 = 1 << 1,
				HD  = 1 << 2,
				NR  = 1 << 3,
				EC  = 1 << 4,
				SE  = 1 << 5,
				IC1 = 1 << 6,
				IC2 = 1 << 7
			};

			enum ST1_FLAGS
			{
				MA = 1 << 0,
				NW = 1 << 1,
				ND = 1 << 2,
				OR = 1 << 4,
				DE = 1 << 5,
				EN = 1 << 7
			};

			enum TIMER
			{
				STEP_RATE_TIME   /*time interval between adjacent step pulse*/,
				HEAD_UNLOAD_TIME /*Time of the end of the execution phase of one of the read/write command to the head unload state*/,
				HEAD_LOAD_TIME   /*Time that the heads take to be loaded after a head load signal*/
			};

		public:
			PIC& picConnected;
			//7654 motors of the drives - 3 enable DMA and IO on the FDC - 10 select one drive if the motors is on
			/**
			 * {0,1} select one drive if its motor is on
			 * 2 - enable/disable FDC
			 * 3 - enable/disable IO and DMA request
			 * {4,5,6,7} - enable/disbale motor of drive #
			 */
			uint8_t dorRegister;
			
			/**
			 * {0,1,2,3} - FDD number # is in seek mode
			 * 4 - FDC is busy
			 * 5 - non-DMA mode
			 * 6 - data transfer direction : {0: CPU --> Resgister, 1: Register --> CPU}
			 * 7 - data register ready to send or receive datas
			 */
			uint8_t statusRegister;
			uint8_t dataRegister;

			std::array<uint8_t, 4> STRegisters;
			std::array<Floppy100,4> floppyDrives;
			std::array<uint8_t, 3> timers;

			std::array<uint8_t,9> configurationDatas;
			std::array<uint8_t,7> resultDatas;

			PHASE phase;
			unsigned int passClock;
			unsigned int clockFromCommandStart;
			bool statusRegisterRead;
			bool readCommandInProcess;
			uint8_t readFromFloppy;

			static FDC* instance;
			static const PIC::IR_LINE ConnectedIRLine = PIC::IR6;
		
		private:
			virtual void write (const unsigned int localAddress, const uint8_t data) final;
			virtual uint8_t read (const unsigned int localAddress) final;
	};
}

#endif