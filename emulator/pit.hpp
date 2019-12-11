#ifndef PIT_HPP
#define PIT_HPP

#include <array>

#include "pic.hpp"
#include "component.hpp"

namespace E5150
{
	class PIT: public Component
	{
		public:
			PIT(PIC& connectedPIC);

		public:
			virtual uint8_t read (const unsigned int address) final;
			virtual void write (const unsigned int address, const uint8_t data) final;
		
		private:
			void writeControlWord (const uint8_t data);
			void setModeForCounter (const unsigned int counterIndex, const uint8_t controlWord);
			void setOperationAccessForCounter (const unsigned int counterIndex, const uint8_t controlWord);
		
		private:
			enum class ACCESS_OPERATION
			{ LATCHING, MSB_ONLY, LSB_ONLY, LSB_MSB };

			enum class MODE
			{ MODE0, MODE1, MODE2, MODE3, MODE4, MODE5 };

			struct Register
			{
				uint16_t value;
				uint16_t latchedValue;
				MODE mode;
				ACCESS_OPERATION accessOperation;
				bool codedBCD;
			};

		private:
			std::array<Register, 3> m_counters;
			PIC& m_connectedPIC;
	};
}

#endif