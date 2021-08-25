#include "biu.hpp"
#include "arch.hpp"

using namespace E5150::I8086;

static constexpr unsigned int BUS_CYCLE_CLOCK = 5;
static unsigned int BUS_CYCLE_CLOCK_LEFT = BUS_CYCLE_CLOCK;

static void instructionFetchClock(void);

static void BIUWaitEndOfControlTransfertInstructionClock(void)
{
	if (BUS_CYCLE_CLOCK_LEFT > 0)
	{
		BUS_CYCLE_CLOCK_LEFT -= 1;

		/*#ifdef DEBUG_BUILD
			printf("BIU: ENDING BUS CYCLE %d (clock count down: %d) --- FETCHING %#5x (%#4x:%#4x)\n", BUS_CYCLE_CLOCK - BUS_CYCLE_CLOCK_LEFT, BUS_CYCLE_CLOCK_LEFT,cpu.genAddress(cpu.cs,cpu.ip),cpu.cs,cpu.ip);
		#endif*/
	}

	/*#ifdef DEBUG_BUILD
		printf("BIU: WAITING END OF CONTROL TRANSFERT INSTRUCTION\n");
	#endif*/
}

static void EUDataAccessClock(void)
{
	if (cpu.biu.EUDataAccessClockCountDown > 0)
	{
		/*#ifdef DEBUG_BUILD
			printf("BIU: DATA ACCESS FROM EU: clock left: %d\n", cpu.biu.EUDataAccessClockCountDown);
		#endif*/
		cpu.biu.EUDataAccessClockCountDown -= 1;
		return;
	}

	cpu.biu.clock = instructionFetchClock;
}

static void waitPlaceInInstructionBufferQueueClock(void)
{
	if (cpu.biu.instructionBufferQueuePos >= 5)
	{
		/*#ifdef DEBUG_BUILD
			printf("BIU: INSTRUCTION BUFFER QUEUE FULL\n");
		#endif*/
		return;
	}

	cpu.biu.clock = instructionFetchClock;
}

static void instructionFetchClock(void)
{
	if (BUS_CYCLE_CLOCK_LEFT > 1)
	{
		BUS_CYCLE_CLOCK_LEFT -= 1;
		/*#ifdef DEBUG_BUILD
			printf("BIU: BUS CYCLE %d (clock count down: %d) --- FETCHING %#5x (%#4x:%#4x)\n", BUS_CYCLE_CLOCK - BUS_CYCLE_CLOCK_LEFT, BUS_CYCLE_CLOCK_LEFT,cpu.genAddress(cpu.cs,cpu.ip),cpu.cs,cpu.ip);
		#endif*/
		return;
	}
	
	if (cpu.biu.instructionBufferQueuePos < 5)
	{
		const unsigned int address = cpu.genAddress(cpu.cs, cpu.ip);
		cpu.biu.instructionBufferQueue[cpu.biu.instructionBufferQueuePos] = ram.read(address);
		cpu.ip += 1;
		cpu.biu.instructionBufferQueuePos += 1;

		/*#ifdef DEBUG_BUILD
			printf("BIU: INSTRUCTION BUFFER QUEUE: queue size %d\n", cpu.biu.instructionBufferQueuePos);

			printf("Instruction buffer: ");
			for (uint8_t b: cpu.biu.instructionBufferQueue)
				printf("%#x ",b);
			putchar('\n');
		#endif*/
	}
	
	if (cpu.biu.instructionBufferQueuePos >= 5)
		cpu.biu.clock = waitPlaceInInstructionBufferQueueClock;
	
	if (cpu.biu.EUDataAccessClockCountDown > 0)
		cpu.biu.clock = EUDataAccessClock;
	
	BUS_CYCLE_CLOCK_LEFT = BUS_CYCLE_CLOCK;
}

BIU::BIU(): clock(instructionFetchClock), EUDataAccessClockCountDown(0) {}

void BIU::endControlTransferInstruction (const uint16_t newCS, const uint16_t newIP)
{
	cpu.cs = newCS;
	cpu.ip = newIP;
	clock = instructionFetchClock;
	BUS_CYCLE_CLOCK_LEFT = BUS_CYCLE_CLOCK;
}

void BIU::startControlTransferInstruction()
{
	clock = BIUWaitEndOfControlTransfertInstructionClock;

	/**
	 * BIU.instructionFetchClock always starts a new bus cycle.
	 * The decoding of the instruction is done in EU.clock, called after BIU.instructionFetchClock
	 * 
	 * If a control transfert instruction is detected and BIU.instructionFetchClock started a new bus cycle, we want to cancel this bus cycle because in real hardware the BIU and EU runs concurrently and the control transfert instruction will be detected before the new bus cycle is launched
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

uint8_t BIU::EURequestReadByte (const unsigned int address)
{ EUDataAccessClockCountDown += 5; return ram.read(address); }
uint16_t BIU::EURequestReadWord (const unsigned int address)
{ return (EURequestReadByte(address+1) << 8) | EURequestReadByte(address); }

void BIU::EURequestWriteByte (const unsigned int address, const uint8_t data)
{ EUDataAccessClockCountDown += 5; ram.write(address, data); }
void BIU::EURequestWriteWord (const unsigned int address, const uint16_t data)
{ EURequestWriteByte(address,data & 0xFF); EURequestWriteByte(address+1, data << 8); }

//TODO: implement port with BIU
uint8_t BIU::EURequestINByte (const unsigned int address)
{ return 0; }
uint16_t BIU::EURequestINWord (const unsigned int address)
{ return 0; }
void BIU::EURequestOUTByte (const unsigned int address, const uint8_t data)
{}
void BIU::EURequestOUTWord (const unsigned int address, const uint16_t data)
{}
