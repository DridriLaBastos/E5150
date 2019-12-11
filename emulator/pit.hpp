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
			PIT(PORTS& ports, PIC& connectedPIC);

		public:
			virtual uint8_t read (const unsigned int address) final;
			virtual void write (const unsigned int address, const uint8_t data) final;
			void clock (void);
		
		private:
			enum class ACCESS_OPERATION
			{ MSB_ONLY, LSB_ONLY, LSB_MSB };

			enum class MODE
			{ MODE0, MODE1, MODE2, MODE3, MODE4, MODE5 };

			enum class OPERATION_STATUS
			{ MSB, LSB };

			enum class OUTPUT_VALUE
			{ HIGH, LOW };

			union CounterValue
			{
				uint16_t word;
				uint16_t msb:8, lsb:8;
			};

			struct Counter
			{
				CounterValue counterValue;
				uint16_t latchedValue;
				MODE mode;
				ACCESS_OPERATION accessOperation;
				OPERATION_STATUS readStatus;
				OPERATION_STATUS writeStatus;
				OUTPUT_VALUE outputValue;
				bool codedBCD;
				bool isCounting;
			};
		
		private:
			void writeControlWord (const uint8_t data);
			void writeCounter (const unsigned int counterIndex, const uint8_t data);
			void setModeForCounter (const unsigned int counterIndex, const uint8_t controlWord);
			void setOperationAccessForCounter (const unsigned int counterIndex, const uint8_t controlWord);

			void execModeForCounter (Counter& counter);

			void clockForCounter0 (void);
			void clockForCounter1 (void);
			void clockForCounter2 (void);
			void clockMode0 (Counter& counter);
			void clockMode1 (Counter& counter);
			void clockMode2 (Counter& counter);
			void clockMode3 (Counter& counter);
			void clockMode4 (Counter& counter);
			void clockMode5 (Counter& counter);

		private:
			std::array<Counter, 3> m_counters;
			PIC& m_connectedPIC;
	};
}

#endif