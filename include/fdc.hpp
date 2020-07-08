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

			enum STATUS_REGISTER_MASK
			{
				D0 = 1 << 0,
				D1 = 1 << 1,
				D2 = 1 << 2,
				D3 = 1 << 3,
				D4 = 1 << 4,
				D5 = 1 << 5,
				D6 = 1 << 6,
				D7 = 1 << 7
			};

			class Command
			{
				public:
					Command (const unsigned int configurationWorldNumber=9, const unsigned int resultWorldNumber=5);

				public:
					//Return true when configuring is done
					bool configure (const uint8_t data);

					//Return true when reading result is done
					std::pair<uint8_t,bool> readResult (void);

					unsigned int exec (void);

				protected:
					//TODO: I don't like having vectors here
					std::vector<uint8_t> m_configurationWords;
					std::vector<uint8_t> m_resultWords;
					unsigned int m_waitTime;
					unsigned int m_floppyDrive;
					unsigned int m_configurationStep;
				
				private:
					virtual void onConfigureFinish (void);
					virtual void onExec (void){}
				
			};

			class COMMAND
			{
				public:
				class ReadData: public Command
				{
					void loadHeads(void);
					void waitHeadSettling(void);
					virtual void onExec (void) final;
					//virtual void onConfigureFinish(void) final;
					enum class STATUS
					{
						LOADING_HEADS, WAIT_HEAD_SETTLING
					};

					STATUS m_status { STATUS::LOADING_HEADS };
				};
				class ReadDeletedData: public Command {};
				class ReadATrack: public Command {};
				class ReadID: public Command {};
				class FormatTrack: public Command {};
				class ScanEqual: public Command {};
				class WriteData: public Command {};
				class WriteDeletedData: public Command {};
				class ScanLEQ: public Command {};
				class ScanHEQ: public Command {};
				class Recalibrate: public Command {};
				class SenseInterruptStatus: public Command {};
				class Specify: public Command{ public: Specify(void); private: virtual void onConfigureFinish(void) final; };
				class SenseDriveStatus: public Command { public: SenseDriveStatus(void); };
				class Seek: public Command { public: Seek(void); };
				class Invalid: public Command { public: Invalid(void); };
			};

			enum TIMER
			{ STEP_RATE_TIME, HEAD_UNLOAD_TIME, HEAD_LOAD_TIME };
	
		private:
			unsigned int onClock (void);
			void switchToCommandMode (void);
			void switchToExecutionMode (void);
			void switchToResultMode (void);

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
			uint8_t m_dorRegister;
			uint8_t m_statusRegister;
			uint8_t m_dataRegister;

			std::array<uint8_t, 4> m_STRegisters;
			std::array<Floppy,4> m_floppyDrives;
			std::array<Command*, 16> m_commands;
			std::array<uint8_t, 3> m_timers;

			PHASE m_phase;
			bool m_statusRegisterRead;
			unsigned int m_selectedCommand;
	};
}

#endif