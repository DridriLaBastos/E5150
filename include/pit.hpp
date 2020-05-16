#ifndef PIT_HPP
#define PIT_HPP

#include "util.hpp"

#include "pic.hpp"
#include "component.hpp"

namespace E5150
{
	class PIT: public Component
	{
		public:
			PIT(PORTS& ports, PIC& connectedPIC);

		public:
			void clock (void);
		
			enum class COUNTER
			{ COUNTER0 = 0, COUNTER1, COUNTER2 };

			void toggleGateFor(const COUNTER counterNUmber);
		
		protected:
			virtual uint8_t read (const unsigned int localAddress) final;
			virtual void write (const unsigned int localAddress, const uint8_t data) final;
		
		private:
			enum class ACCESS_OPERATION
			{ MSB_ONLY, LSB_ONLY, LSB_MSB };

			enum class OPERATION_STATUS
			{ MSB, LSB };

			enum class OUTPUT_VALUE
			{ HIGH, LOW };

			union CounterValue
			{
				uint16_t word;
				uint16_t msb:8, lsb:8;
			};

			class MODE;

			struct Counter
			{
				CounterValue counterValue;
				uint16_t latchedValue;
				MODE* mode;
				ACCESS_OPERATION accessOperation;
				OPERATION_STATUS readStatus;
				OPERATION_STATUS writeStatus;
				OUTPUT_VALUE outputValue;
				OUTPUT_VALUE gateValue;
				bool codedBCD;
				bool isCounting;
				bool latchedValueIsAvailable;
				bool readComplete;
				bool waitClock;
				unsigned int counterIndex;//Used for messages
			};

			class MODE
			{
				public:
					virtual void enable (Counter& counter)	= 0;
					virtual void clock (Counter& counter)	= 0;
					virtual void writeOperation (Counter& counter, const uint8_t count) = 0;
			};

			class MODE0: public MODE
			{
				public:
					virtual void enable (Counter& counter) final;
					virtual void clock (Counter& counter) final;
					virtual void writeOperation (Counter& counter, const uint8_t count) final;
			};

			class MODE1: public MODE
			{
				public:
					virtual void enable (Counter& counter) final;
					virtual void clock (Counter& counter) final;
					virtual void writeOperation (Counter& counter, const uint8_t count) final;
			};

			class MODE2: public MODE
			{
				public:
					virtual void enable (Counter& counter) final;
					virtual void clock (Counter& counter) final;
					virtual void writeOperation (Counter& counter, const uint8_t count) final;
			};

			class MODE3: public MODE
			{
				public:
					virtual void enable (Counter& counter) final;
					virtual void clock (Counter& counter) final;
					virtual void writeOperation (Counter& counter, const uint8_t count) final;
			};

			class MODE4: public MODE
			{
				public:
					virtual void enable (Counter& counter) final;
					virtual void clock (Counter& counter) final;
					virtual void writeOperation (Counter& counter, const uint8_t count) final;
			};

			class MODE5: public MODE
			{
				public:
					virtual void enable (Counter& counter) final;
					virtual void clock (Counter& counter) final;
					virtual void writeOperation (Counter& counter, const uint8_t count) final;
			};
		
		private:
			void writeControlWord (const uint8_t data);
			void writeCounter (const unsigned int counterIndex, const uint8_t data);
			void setOperationAccessForCounter (const unsigned int counterIndex, const uint8_t controlWord);

			void setModeForCounter (const unsigned int counterIndex, const uint8_t controlWord);

			void clockForCounter0 (void);
			void clockForCounter1 (void);
			void clockForCounter2 (void);

			uint8_t readCounterDirectValue	(Counter& counter);
			uint8_t readCounterLatchedValue	(Counter& counter);
			uint8_t applyPICReadAlgorithm	(Counter& counter, const unsigned int value);
		
		private:
			/* Declaration of all the modes so that their addresses can be taken when setting a mode */
			static E5150::PIT::MODE0 mode0;
			static E5150::PIT::MODE1 mode1;
			static E5150::PIT::MODE2 mode2;
			static E5150::PIT::MODE3 mode3;
			static E5150::PIT::MODE4 mode4;
			static E5150::PIT::MODE5 mode5;

		private:
			std::array<Counter, 3> m_counters;
			PIC& m_connectedPIC;
	};
}

#endif