#include "biu.hpp"
#include "arch.hpp"

using namespace E5150::I8086;

static constexpr unsigned int BUS_CYCLE_CLOCK = 4;
static unsigned int BUS_CYCLE_CLOCK_LEFT = BUS_CYCLE_CLOCK;

static void BIUInstructionFetchClock(void);

static void BIUWaitEndOfControlTransfertInstructionClock(void)
{
	if (BUS_CYCLE_CLOCK_LEFT > 0)
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
		printf("BIU: DATA ACCESS FROM EU: clock left: %d\n", cpu.biu.EUMemoryAccessClockCountDown);
	#endif
	cpu.biu.EUMemoryAccessClockCountDown -= 1;
}

static void BIUWaitPlaceInInstrutionBufferQueueClock(void)
{
	printf("REACHED WAIT %d\n",cpu.biu.instructionBufferQueuePos);
	if (cpu.biu.instructionBufferQueuePos >= 5)
	{
		#ifdef DEBUG_BUILD
			printf("BIU: INSTRUCTION BUFFER QUEUE FULL\n");
		#endif
		return;
	}
}

static void BIUInstructionFetchClock(void)
{
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

	BUS_CYCLE_CLOCK_LEFT -= 1;
	#ifdef DEBUG_BUILD
		printf("BIU: BUS CYCLE %d (clock count down: %d) --- FETCHING %#5x (%#4x:%#4x)\n", BUS_CYCLE_CLOCK - BUS_CYCLE_CLOCK_LEFT, BUS_CYCLE_CLOCK_LEFT,cpu.genAddress(cpu.cs,cpu.ip),cpu.cs,cpu.ip);
	#endif
}

BIU::BIU(): clock(BIUInstructionFetchClock), nextClockFunction(BIUInstructionFetchClock), EUMemoryAccessClockCountDown(0),EUExecutesControlTransfertInstruction(false) {}

void BIU::updateClockFunction()
{
	clock = BIUInstructionFetchClock;

	if (cpu.biu.instructionBufferQueuePos >= 5)
		clock = BIUWaitPlaceInInstrutionBufferQueueClock;
	
	if (cpu.biu.EUExecutesControlTransfertInstruction)
		clock = BIUWaitEndOfControlTransfertInstructionClock;

	if (cpu.biu.EUMemoryAccessClockCountDown > 0)
		clock = BIUDataAccessClock;
}

void BIU::endControlTransferInstruction ()
{
	resetInstructionBufferQueue();
	BUS_CYCLE_CLOCK_LEFT = BUS_CYCLE_CLOCK;
	cpu.biu.EUExecutesControlTransfertInstruction = false;
}

void BIU::startControlTransferInstruction()
{
	cpu.biu.EUExecutesControlTransfertInstruction = true;

	/**
	 * BIU.BIUInstructionFetchClock always starts a new bus cycle.
	 * The decoding of the instruction is done in EU.clock, called after BIU.BIUInstructionFetchClock
	 * 
	 * If a control transfert instruction is detected and BIU.BIUInstructionFetchClock started a new bus cycle, we want to cancel this bus cycle because in real hardware the BIU and EU runs concurrently and the control transfert instruction will be detected before the new bus cycle is launched
	 */
	if (BUS_CYCLE_CLOCK_LEFT == BUS_CYCLE_CLOCK)
		BUS_CYCLE_CLOCK_LEFT = 0;
}

void BIU::instructionBufferQueuePop(const unsigned int n)
{
	for (size_t i = 0; i+n < 5; i++)
		instructionBufferQueue[i] = instructionBufferQueue[i+n];
	instructionBufferQueuePos -= n;
}

void BIU::resetInstructionBufferQueue(){ instructionBufferQueuePos = 0; }
void BIU::requestMemoryByte(const unsigned int nBytes) noexcept { EUMemoryAccessClockCountDown += nBytes * BUS_CYCLE_CLOCK; }

uint8_t BIU::readByte (const unsigned int address) const { return ram.read(address); }
uint16_t BIU::readWord (const unsigned int address) const { return (readByte(address + 1) << 8) | readByte(address); }

void BIU::writeByte (const unsigned int address, const uint8_t byte) const { ram.write(address, byte); }
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
