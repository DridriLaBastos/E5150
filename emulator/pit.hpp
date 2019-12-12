#ifndef PIT_HPP
#define PIT_HPP

#include "pic.hpp"
#include "util.hpp"
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
				bool codedBCD;
				bool isCounting;
				bool latchedValueIsAvailable;
				bool readComplete;
			};

			enum class COUNTER
			{ COUNTER0 = 0, COUNTER1, COUNTER2 };

			class MODE
			{
				public:
					void setForCounter (Counter& counter);
					virtual void clock (Counter& counter) = 0;
					virtual void writeToCounter (Counter& counter, const uint8_t count) = 0;
				
				private:
					virtual void actionForSet (Counter& counter) = 0;
			};

			class MODE0: public MODE
			{
				public:
					virtual void clock (Counter& counter) final;
					virtual void writeToCounter (Counter& counter, const uint8_t count) final;
				
				private:
					virtual void actionForSet (Counter& counter) final;
			};

			class MODE1: public MODE
			{
				public:
					virtual void clock (Counter& counter) final;
					virtual void writeToCounter (Counter& counter, const uint8_t count) final;
				
				private:
					virtual void actionForSet (Counter& counter) final;
			};

			class MODE2: public MODE
			{
				public:
					virtual void clock (Counter& counter) final;
					virtual void writeToCounter (Counter& counter, const uint8_t count) final;
				
				private:
					virtual void actionForSet (Counter& counter) final;
			};

			class MODE3: public MODE
			{
				public:
					virtual void clock (Counter& counter) final;
					virtual void writeToCounter (Counter& counter, const uint8_t count) final;
				
				private:
					virtual void actionForSet (Counter& counter) final;
			};

			class MODE4: public MODE
			{
				public:
					virtual void clock (Counter& counter) final;
					virtual void writeToCounter (Counter& counter, const uint8_t count) final;
				
				private:
					virtual void actionForSet (Counter& counter) final;
			};

			class MODE5: public MODE
			{
				public:
					virtual void clock (Counter& counter) final;
					virtual void writeToCounter (Counter& counter, const uint8_t count) final;
				
				private:
					virtual void actionForSet (Counter& counter) final;
			};
		
		private:
			void writeControlWord (const uint8_t data);
			void writeCounter (const unsigned int counterIndex, const uint8_t data);
			void setOperationAccessForCounter (const unsigned int counterIndex, const uint8_t controlWord);

			void setMode0ForCounter (Counter& counter);
			void setMode1ForCounter (Counter& counter);
			void setMode2ForCounter (Counter& counter);
			void setMode3ForCounter (Counter& counter);
			void setMode4ForCounter (Counter& counter);
			void setMode5ForCounter (Counter& counter);
			void setModeForCounter (const unsigned int counterIndex, const uint8_t controlWord);

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

			uint8_t readCounterDirectValue	(Counter& counter);
			uint8_t readCounterLatchedValue	(Counter& counter);
			uint8_t applyPICReadAlgorithm	(Counter& counter, const unsigned int value);

		private:
			std::array<Counter, 3> m_counters;
			PIC& m_connectedPIC;
			MODE0 m_mode0;
			MODE1 m_mode1;
			MODE2 m_mode2;
			MODE3 m_mode3;
			MODE4 m_mode4;
			MODE5 m_mode5;
	};
}

#endif