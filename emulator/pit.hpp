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
		
			enum class COUNTER
			{ COUNTER0 = 0, COUNTER1, COUNTER2 };

			void toggleGateFor(const COUNTER counterNUmber);
		
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
			};

			class MODE
			{
				public:
					MODE(Counter& relatedCounter);

				public:
					void enable (void);
					virtual void clock (void);
					virtual void writeOperation (const uint8_t count);
				
				private:
					virtual void actionOnEnable (void);
				
				protected:
					Counter& m_relatedCounter;
			};

			class MODE0: public MODE
			{
				public:
					virtual void clock (void) final;
					virtual void writeOperation (const uint8_t count) final;
				
				private:
					virtual void actionOnEnable (void) final;
			};

			class MODE1: public MODE
			{
				public:
					virtual void clock (void) final;
					virtual void writeOperation (const uint8_t count) final;
				
				private:
					virtual void actionOnEnable (void) final;
			};

			class MODE2: public MODE
			{
				public:
					virtual void clock (void) final;
					virtual void writeOperation (const uint8_t count) final;
				
				private:
					virtual void actionOnEnable (void) final;
			};

			class MODE3: public MODE
			{
				public:
					virtual void clock (void) final;
					virtual void writeOperation (const uint8_t count) final;
				
				private:
					virtual void actionOnEnable (void) final;
			};

			class MODE4: public MODE
			{
				public:
					virtual void clock (void) final;
					virtual void writeOperation (const uint8_t count) final;
				
				private:
					virtual void actionOnEnable (void) final;
			};

			class MODE5: public MODE
			{
				public:
					virtual void clock (void) final;
					virtual void writeOperation (const uint8_t count) final;
				
				private:
					virtual void actionOnEnable (void) final;
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
			std::array<Counter, 3> m_counters;
			PIC& m_connectedPIC;
			std::array<std::array<std::unique_ptr<MODE>, 3>, 6> m_modes;
	};
}

#endif