#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <utility>

#include "pic.hpp"
#include "util.hpp"
#include "ports.hpp"
#include "component.hpp"

namespace E5150
{
	/**
	 * This class will emulate the floppy disc controller in the IBM and the behaviour of the DOR register
	 */
	class Floppy: public Component
	{
		public:
			Floppy(PIC& pic,PORTS& ports);
		
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
					Command(void): m_step(0){}
					//Return true when configuring is done
					virtual bool configure (const uint8_t data) = 0;
					//Return true when reading result is done
					virtual std::pair<uint8_t,bool> readResult (void) = 0;
				
				protected:
					unsigned int m_step;
			};

			class ReadData: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class ReadDeletedData: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class ReadATrack: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class ReadID: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class FormatTrack: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class ScanEqual: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};


			class WriteData: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class WriteDeletedData: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class ScanLEQ: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class ScanHEQ: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class Recalibrate: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class SenseInterruptStatus: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class Specify: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class SenseDriveStat: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};

			class Seek: public Command 
			{
				virtual bool configure (const uint8_t data) final{return false;}
				virtual std::pair<uint8_t, bool> readResult (void) final{return {0,true}; }
			};


			class Invalid: public Command
			{
				virtual bool configure (const uint8_t) final { return false; }
				virtual std::pair<uint8_t,bool> readResult (void) final { return {0x80,true}; }
			};
		
		private:
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
		//have to create instances for each classes (but do we even need multiple classes ? I don't know now yet so
		//we go non static)
		private:
			ReadData readData;
			ReadDeletedData readDeletedData;
			ReadATrack readATrack;
			ReadID readID;
			FormatTrack formatTrack;
			ScanEqual scanEqual;
			WriteData writeData;
			WriteDeletedData writeDeletedData;
			ScanLEQ scanLEQ;
			ScanHEQ scanHEQ;
			Recalibrate recalibrate;
			SenseInterruptStatus senseInterruptStatus;
			Specify specify;
			SenseDriveStat senseDriveStat;
			Seek seek;
			Invalid invalid;

		//Behaviour private space
		private:
			uint8_t m_dorRegister;
			uint8_t m_statusRegister;
			uint8_t m_dataRegister;

			std::array<uint8_t, 4> m_STRegisters;
			std::array<Command*, 16> m_commands;

			PHASE m_phase;
			bool m_statusRegisterRead;
			unsigned int m_selectedCommand;
	};
}

#endif