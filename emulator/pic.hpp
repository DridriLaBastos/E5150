#ifndef PIC_HPP
#define PIC_HPP

#include "util.hpp"

#include "8086.hpp"
#include "ports.hpp"
#include "component.hpp"

namespace E5150
{
	class PIC: public E5150::Component
	{
		public:
			PIC (PORTS& ports, CPU& connectedCPU);
		
		public:
			enum INTERRUPT_LINE
			{
				IR0 = 1 << 0,
				IR1 = 1 << 1,
				IR2 = 1 << 2,
				IR3 = 1 << 3,
				IR4 = 1 << 4,
				IR5 = 1 << 5,
				IR6 = 1 << 6,
				IR7 = 1 << 7
			};
		
		public:
			virtual void write		(const unsigned int address, const uint8_t data) final;
			virtual uint8_t read	(const unsigned int address) final;

			void assertInteruptLine (const INTERRUPT_LINE irLine);
		
		private:

			uint8_t readA0_0 (void) const;
			uint8_t readA0_1 (void) const;

			void writeA0_0 (const uint8_t data);
			void writeA0_1 (const uint8_t data);

			void handleICW1 (const uint8_t icw1);
			void handleICW2 (const uint8_t icw2);
			void handleICW3 (const uint8_t icw3);
			void handleICW4 (const uint8_t icw4);
			void handleInitSequence (const uint8_t icw);

			void handleOCW1 (const uint8_t ocw1);
			void handleOCW2 (const uint8_t ocw2);
			void handleOCW3 (const uint8_t ocw3);

			void nonSpecificEOI(void);
			void specificEOI (const unsigned int IRLevelToBeActedUpon);

			//interruptLineNumber is 0 for IR0, 1 for IR1, 2 for IR2 etc
			void interruptInFullyNestedMode (const unsigned int interruptLineNumber);

			void reInit(void);
			void rotatePriorities(const unsigned int pivot);
		
		private:
			enum REGISTER
			{ ISR = 0, IMR, IRR };

			enum class INIT_STATUS
			{ UNINITIALIZED, ICW2, ICW3, ICW4, INITIALIZED };

			enum class BUFFERED_MODE
			{ NON, SLAVE, MASTER };

			struct PicInfo
			{
				bool icw4Needed;
				bool singleMode;
				bool addressInterval4;
				bool levelTriggered;
				bool isMaster;
				unsigned int T7_T3;
				unsigned int slaveID;
				//bool mode8086_8088; The PIC MUST be in 8086/8088 mode in the IBM PC
				bool autoEOI;
				BUFFERED_MODE bufferedMode;
				bool specialFullyNestedMode;
			};

		private:
			std::array<uint8_t, 3> m_regs;
			std::array<uint8_t, 8> m_priorities;
			CPU& m_connectedCPU;
			INIT_STATUS m_initStatus;
			PicInfo m_picInfo;
			REGISTER m_nextRegisterToRead;
			unsigned int m_IRLineWithPriority0;
	};
}

#endif