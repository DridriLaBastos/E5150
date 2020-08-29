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
			enum IR_LINE
			{ IR0 = 0,IR1,IR2,IR3,IR4,IR5,IR6,IR7 };

			enum REGISTER
			{ ISR = 0, IMR, IRR };

			enum class INIT_STATUS
			{ UNINITIALIZED, ICW2, ICW3, ICW4, INITIALIZED };

			enum class BUFFERED_MODE
			{ NON, SLAVE, MASTER };
		
		public:
			void assertInterruptLine (const IR_LINE IRLine,const Component* caller);
		
		protected:
			virtual void write		(const unsigned int localAddress, const uint8_t data) final;
			virtual uint8_t read	(const unsigned int localAddress) final;

			struct PicInfo
			{
				bool icw4Needed;
				bool singleMode;
				bool levelTriggered;
				bool isMaster;
				unsigned int firstInterruptVector;
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
			CPU& connectedCPU;
	};
}

#endif