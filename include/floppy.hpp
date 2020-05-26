#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "pic.hpp"
#include "util.hpp"
#include "ports.hpp"
#include "component.hpp"

namespace E5150
{
	class Floppy: public Component
	{
		public:
			Floppy(PIC& pic,PORTS& ports);
		
		private:
			void writeDOR(const uint8_t data);
			void writeDataRegister(const uint8_t data);
		
		private:
			virtual void write		(const unsigned int localAddress, const uint8_t data) final;
			virtual uint8_t read	(const unsigned int localAddress) final;

		//Connection private space
		private:
			PIC& m_pic;

		//Behaviour private space
		private:
			uint8_t m_dor;
			uint8_t m_mainStatusRegister;//read only
			std::array<uint8_t, 4> m_dataRegisters;
			unsigned int m_mainDataRegisterIndex;
	};
}

#endif