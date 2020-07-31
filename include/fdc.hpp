#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "pic.hpp"
#include "util.hpp"
#include "ports.hpp"
#include "floppy.hpp"
#include "component.hpp"

namespace E5150
{
	/**
	 * This class will emulate the floppy disc controller in the IBM and the behaviour of the DOR register
	 */
	//TODO: at what frequency does the fdc really runs ? I assume 8MHz but it could be 4MHz.
	class FDC: public Component
	{
		public:
			FDC(PIC& pic,PORTS& ports);
		
		public:
			void clock (void);
		
		private:
			enum class PHASE
			{ COMMAND, EXECUTION, RESULT };

			enum ST0_FLAGS
			{
				US0 = 1 << 0,
				US1 = 1 << 1,
				HD  = 1 << 2,
				NR  = 1 << 3,
				EC  = 1 << 4,
				SE  = 1 << 5,
				IC1 = 1 << 6,
				IC2 = 1 << 4
			};

			class Command
			{
				public:
					Command (const std::string& name = "", const unsigned int configurationWorldNumber=9, const unsigned int resultWorldNumber=7);

				public:
					//Return true when configuring is done
					bool configure (const uint8_t data);

					//Return true when reading result is done
					std::pair<uint8_t,bool> readResult (void);

					virtual void exec (const unsigned int fdcClockElapsed = 0);//TODO: should be pure virtual
					const std::string m_name;
					
				protected:
					//TODO: I don't like having vectors here
					std::vector<uint8_t> m_configurationWords;
					std::vector<uint8_t> m_resultWords;
					unsigned int m_clockWait;
					unsigned int m_floppyDrive;
					unsigned int m_configurationStep;
				
					bool m_checkMFM = true;
					bool m_saveHDS_DSx = true;

				private:
					virtual void onConfigureBegin  (void);
					virtual void onConfigureFinish (void);
				
			};

			struct COMMAND
			{
				class ReadData: public Command
				{
					enum class STATUS
					{ LOADING_HEADS, READ_DATA };

					void loadHeads(void);
					virtual void exec () final;
					//virtual void onConfigureFinish(void) final;

					STATUS m_status { STATUS::LOADING_HEADS };
				};
				class ReadDeletedData: public Command {};
				class ReadATrack: public Command {};
				class ReadID: public Command { virtual void exec (void) final; public: ReadID(void); };
				class FormatTrack: public Command {};
				class ScanEqual: public Command {};
				class WriteData: public Command {};
				class WriteDeletedData: public Command {};
				class ScanLEQ: public Command {};
				class ScanHEQ: public Command {};
				class Recalibrate: public Command {};
				class SenseInterruptStatus: public Command {};
				class Specify: public Command{ virtual void onConfigureFinish(void) final;public: Specify(void); };
				class SenseDriveStatus: public Command { public: SenseDriveStatus(void); };
				class Seek: public Command
				{
					public: Seek(void);
					private:
					virtual void exec (const unsigned int fdcClockElapsed) final;
					virtual void onConfigureBegin(void) final;
					virtual void onConfigureFinish(void) final;
					void execOnFloppyDrive (Floppy100& drive) const;

					private:
						void finish(void);

					private:
						bool m_direction;
						bool m_firstStep;
						Floppy100* m_floppyToApply = nullptr;
				};

				class Invalid: public Command { virtual void onConfigureFinish(void) final;; public: Invalid(void); };
			};

			enum TIMER
			{
				STEP_RATE_TIME   /*time interval between adjacent step pulse*/,
				HEAD_UNLOAD_TIME /*Time of the end of the execution phase of one of the read/write command to the head unload state*/,
				HEAD_LOAD_TIME   /*Time that the heads take to be loaded after a head load signal*/
			};
	
		private:
			void waitClock (const unsigned int clock);
			void waitMicro (const unsigned int microseconds);
			void waitMilli (const unsigned int milliseconds);
			
			void makeBusy (void);
			void makeAvailable (void);

			void setSeekStatusOn(const unsigned int driveNumber);
			void resetSeekStatusOf (const unsigned int driveNumber);

			void switchToCommandMode   (void);
			void switchToExecutionMode (void);
			void switchToResultMode    (void);

			void makeDataRegisterReady (void);
			void makeDataRegisterNotReady (void);
			void makeDataRegisterInReadMode (void);
			void makeDataRegisterInWriteMode (void);

			bool dataRegisterReady (void) const;
			bool dataRegisterInReadMode (void) const;
			bool dataRegisterInWriteMode (void) const;

			bool statusRegisterAllowReading (void) const;
			bool statusRegisterAllowWriting (void) const;

			void setST0Flag (const ST0_FLAGS flag);
			void resetST0Flag (const ST0_FLAGS flag);

			uint8_t readDataRegister(void);
			uint8_t readStatusRegister (void);
			virtual uint8_t read (const unsigned int localAddress) final;

			void writeDOR(const uint8_t data);
			void writeDataRegister(const uint8_t data);
			virtual void write		(const unsigned int localAddress, const uint8_t data) final;

			void switchPhase (void);

		//Connection private space
		private:
			PIC& m_pic;
		
		//Commands
		//TODO: maybe they should become static members because all the commands are the same for FDCs so we don't
		//have to create instances for each classes (but do we even need multiple classes ? I don't know yet so
		//we go non static)
		private:
			COMMAND::ReadData readData;
			COMMAND::ReadDeletedData readDeletedData;
			COMMAND::ReadATrack readATrack;
			COMMAND::ReadID readID;
			COMMAND::FormatTrack formatTrack;
			COMMAND::ScanEqual scanEqual;
			COMMAND::WriteData writeData;
			COMMAND::WriteDeletedData writeDeletedData;
			COMMAND::ScanLEQ scanLEQ;
			COMMAND::ScanHEQ scanHEQ;
			COMMAND::Recalibrate recalibrate;
			COMMAND::SenseInterruptStatus senseInterruptStatus;
			COMMAND::Specify specify;
			COMMAND::SenseDriveStatus senseDriveStat;
			COMMAND::Seek seek;
			COMMAND::Invalid invalid;

		//Behaviour private space
		private:
			//7654 motors of the drives - 3 enable DMA and IO on the FDC - 10 select one drive if the motors is on
			/**
			 * {0,1} select one drive if its motor is on
			 * 2 - enable/disable FDC
			 * 3 - enable/disable IO and DMA request
			 * {4,5,6,7} - enable/disbale motor of drive #
			 */
			uint8_t m_dorRegister;
			
			/**
			 * {0,1,2,3} - FDD number # is in seek mode
			 * 4 - FDC is busy
			 * 5 - non-DMA mode
			 * 6 - data transfer direction : {0: CPU --> Resgister, 1: Register --> CPU}
			 * 7 - data register ready to send or receive datas
			 */
			uint8_t m_statusRegister;
			uint8_t m_dataRegister;

			std::array<uint8_t, 4> m_STRegisters;
			std::array<Floppy100,4> m_floppyDrives;
			std::array<Command*, 16> m_commands;
			std::array<uint8_t, 3> m_timers;

			unsigned int cnc = 0;

			PHASE m_phase;
			unsigned int m_selectedCommand;
			unsigned int m_passClock;
			bool m_statusRegisterRead;
	};
}

#endif