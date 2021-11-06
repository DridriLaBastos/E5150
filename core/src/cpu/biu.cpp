#include "biu.hpp"
#include "arch.hpp"

using namespace E5150::I8086;

static constexpr unsigned int BUS_CYCLE_CLOCK = 4;
static unsigned int BUS_CYCLE_CLOCK_LEFT = BUS_CYCLE_CLOCK;
static unsigned int EU_DATA_ACCESS_CLOCK_LEFT = 0;
static bool EU_EXECUTES_CONTROL_TRANSFERT_INSTRUCTION = false;

static void BIUInstructionFetchClock(void);

static void BIUWaitEndOfControlTransfertInstructionClock(void)
{
	if (BUS_CYCLE_CLOCK_LEFT < BUS_CYCLE_CLOCK)
	{
		BUS_CYCLE_CLOCK_LEFT -= 1;

		#ifdef DEBUG_BUILD
			printf("BIU: ENDING BUS CYCLE %d (clock count down: %d) --- FETCHING %#5x (%#4x:%#4x)\n", BUS_CYCLE_CLOCK - BUS_CYCLE_CLOCK_LEFT, BUS_CYCLE_CLOCK_LEFT,cpu.genAddress(cpu.cs,cpu.ip),cpu.cs,cpu.ip);
		#endif
		return;
	}

	#ifdef DEBUG_BUILD
		printf("BIU: WAITING END OF CONTROL TRANSFERT INSTRUCTION\n");
	#endif
}

static void BIUDataAccessClock(void)
{
	#ifdef DEBUG_BUILD
		printf("BIU: DATA ACCESS FROM EU: clock left: %d\n", EU_DATA_ACCESS_CLOCK_LEFT);
	#endif
	EU_DATA_ACCESS_CLOCK_LEFT -= 1;
}

static void BIUWaitPlaceInInstrutionBufferQueueClock(void)
{
	#ifdef DEBUG_BUILD
		printf("BIU: INSTRUCTION BUFFER QUEUE FULL\n");
	#endif
}

static void BIUInstructionFetchClock(void)
{
	BUS_CYCLE_CLOCK_LEFT -= 1;
	#ifdef DEBUG_BUILD
		printf("BIU: BUS CYCLE %d (clock count down: %d) --- FETCHING %#5x (%#4x:%#4x)\n", BUS_CYCLE_CLOCK - BUS_CYCLE_CLOCK_LEFT, BUS_CYCLE_CLOCK_LEFT,cpu.genAddress(cpu.cs,cpu.ip),cpu.cs,cpu.ip);
	#endif

	if (BUS_CYCLE_CLOCK_LEFT == 0)
	{
		if (cpu.biu.instructionBufferQueuePos < 5)
		{
			const unsigned int address = cpu.genAddress(cpu.cs, cpu.ip);
			cpu.biu.instructionBufferQueue[cpu.biu.instructionBufferQueuePos] = ram.read(address);
			cpu.ip += 1;
			cpu.biu.instructionBufferQueuePos += 1;

			#ifdef DEBUG_BUILD
				printf("BIU: INSTRUCTION BUFFER QUEUE: queue size %d\n", cpu.biu.instructionBufferQueuePos);

				printf("Instruction buffer: ");
				for (uint8_t b: cpu.biu.instructionBufferQueue)
					printf("%#x ",b);
				putchar('\n');
			#endif
			BUS_CYCLE_CLOCK_LEFT = BUS_CYCLE_CLOCK;
		}
	}
}

BIU::BIU(): clock(BIUInstructionFetchClock) {}

void BIU::updateClockFunction()
{
	//If a new bus cycle is about to start a new clock function is used to execute custom functions
	if (BUS_CYCLE_CLOCK_LEFT == BUS_CYCLE_CLOCK)
	{
		clock = BIUInstructionFetchClock;

		//No special action are required but the buffer queu is full
		if (cpu.biu.instructionBufferQueuePos >= 5)
			clock = BIUWaitPlaceInInstrutionBufferQueueClock;

		//A control transfert instruction is being executed and no data are required
		//TODO: Is there a case were EU_EXECUTES_CONTROL_TRANSFERT_INSTRUCTION goes from true to false in less than the clock cycles needed to complete the current bus cycle ?
		if (EU_EXECUTES_CONTROL_TRANSFERT_INSTRUCTION)
			clock = BIUWaitEndOfControlTransfertInstructionClock;

		//Biger priority: the EU request data from memory
		if (EU_DATA_ACCESS_CLOCK_LEFT > 0)
			clock = BIUDataAccessClock;
	}
}

void BIU::endControlTransferInstruction (const bool flushInstructionQueue)
{
	if (flushInstructionQueue)
		cpu.eu.instructionLength = 5;
	EU_EXECUTES_CONTROL_TRANSFERT_INSTRUCTION = false;
}

void BIU::startControlTransferInstruction()
{ EU_EXECUTES_CONTROL_TRANSFERT_INSTRUCTION = true; }

void BIU::instructionBufferQueuePop(const unsigned int n)
{
	for (size_t i = 0; i+n < 5; i++)
		instructionBufferQueue[i] = instructionBufferQueue[i+n];
	instructionBufferQueuePos -= n;
}

void BIU::resetInstructionBufferQueue(){ instructionBufferQueuePos = 0; }
void BIU::requestMemoryByte(const unsigned int nBytes) noexcept { EU_DATA_ACCESS_CLOCK_LEFT += nBytes * BUS_CYCLE_CLOCK; }

uint8_t BIU::readByte (const unsigned int address) const { EU_DATA_ACCESS_CLOCK_LEFT += 4; return ram.read(address); }
uint16_t BIU::readWord (const unsigned int address) const { return (readByte(address + 1) << 8) | readByte(address); }

void BIU::writeByte (const unsigned int address, const uint8_t byte) const { EU_DATA_ACCESS_CLOCK_LEFT += 4; ram.write(address, byte); }
void BIU::writeWord (const unsigned int address, const uint16_t word) const { writeByte(address,(uint8_t)word); writeByte(address+1,word >> 8); }

//TODO: Get the clock access count per components
//TODO: Use the data bus to get the value or only keep it for undefined values ?
uint8_t BIU::inByte (const unsigned int address)
{ return ports.read(address); }
uint16_t BIU::inWord (const unsigned int address)
{ return (inByte(address + 1) << 8) | inByte(address); }

void BIU::outByte (const unsigned int address, const uint8_t data)
{ ports.write(address,data); }
void BIU::outWord (const unsigned int address, const uint16_t data)
{ outByte(address,data);   outByte(address,data >> 8); }
