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

			enum REGISTER
			{ ISR = 0, IMR, IRR };

			enum class INIT_STATUS
			{ UNINITIALIZED, ICW2, ICW3, ICW4, INITIALIZED };

			enum class BUFFERED_MODE
			{ NON, SLAVE, MASTER };
		
		public:
			void assertInteruptLine (const INTERRUPT_LINE irLine);
		
		protected:
			virtual void write		(const unsigned int localAddress, const uint8_t data) final;
			virtual uint8_t read	(const unsigned int localAddress) final;

			//interruptLineNumber is 0 for IR0, 1 for IR1, 2 for IR2 etc
			void interruptInFullyNestedMode (const unsigned int interruptLineNumber);

			void rotatePriorities(const unsigned int pivot);

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

		public:
			std::array<uint8_t, 3> regs;
			std::array<uint8_t, 8> priorities;
			INIT_STATUS initStatus;
			PicInfo info;
			unsigned int IRLineWithPriority0;
			REGISTER nextRegisterToRead;

		private:
			CPU& m_connectedCPU;
	};
}

#endif