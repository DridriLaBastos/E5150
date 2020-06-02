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

		//Behaviour private space
		private:
			uint8_t m_dorRegister;
			uint8_t m_statusRegister;
			uint8_t m_dataRegister;

			std::array<uint8_t, 4> m_STRegisters;

			std::array<uint8_t, 5> m_resultCommandData;
			int m_resultCommandDataToRead;

			PHASE m_phase;
	};
}

#endif