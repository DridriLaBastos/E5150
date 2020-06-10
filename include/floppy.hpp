#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

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
					bool virtual configure (const uint8_t data) = 0;
				
				protected:
					unsigned int m_step;
			};

			class ReadData: public Command { virtual bool configure (const uint8_t data) final; };
			class ReadDeletedData: public Command { virtual bool configure (const uint8_t data) final; };
			class ReadATrack: public Command { virtual bool configure (const uint8_t data) final; };
			class ReadID: public Command { virtual bool configure (const uint8_t data) final; };
			class FormatTrack: public Command { virtual bool configure (const uint8_t data) final; };
			class ScanEqual: public Command { virtual bool configure (const uint8_t data) final; };

			class WriteData: public Command { virtual bool configure (const uint8_t data) final; };
			class WriteDeletedData: public Command { virtual bool configure (const uint8_t data) final; };
			class ScanLEQ: public Command { virtual bool configure (const uint8_t data) final; };
			class ScanHEQ: public Command { virtual bool configure (const uint8_t data) final; };
			class Recalibrate: public Command { virtual bool configure (const uint8_t data) final; };
			class SenseInterruptStatus: public Command { virtual bool configure (const uint8_t data) final; };
			class Specify: public Command { virtual bool configure (const uint8_t data) final; };
			class SenseDriveStat: public Command { virtual bool configure (const uint8_t data) final; };
			class Seek: public Command { virtual bool configure (const uint8_t data) final; };

			class Invalid: public Command { virtual bool configure (const uint8_t) final { return false; } };
		
		private:
			bool areStatusBitSet (const STATUS_REGISTER_MASK statusRegisterToTestMask);

			uint8_t readDataRegister(void);
			uint8_t readStatusRegister (void);
			virtual uint8_t read	(const unsigned int localAddress) final;

			void writeDOR(const uint8_t data);
			void writeDataRegister(const uint8_t data);
			virtual void write		(const unsigned int localAddress, const uint8_t data) final;

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
			std::array<uint8_t, 5> m_resultCommandData;
			std::array<Command*, 16> m_commands;
			int m_resultCommandDataToRead;

			PHASE m_phase;
	};
}

#endif