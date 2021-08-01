#include "biu.hpp"
#include "arch.hpp"

using namespace E5150::I8086;

static void instructionFetchClock(void);

static void EUDataAccessClock(void)
{
	if (cpu.biu.EUDataAccessClockCountDown > 0)
	{
		printf("BIU: DATA ACCESS FROM EU: clock left: %d\n", cpu.biu.EUDataAccessClockCountDown);
		cpu.biu.EUDataAccessClockCountDown -= 1;
		return;
	}
	
	cpu.biu.clock = instructionFetchClock;
}

static void instructionFetchClock(void)
{
	static unsigned int clockCountDown = 5;
	
	if (clockCountDown > 0)
	{
		printf("BIU: BUS CYCLE %d (clock count down: %d) --- FETCHING %#5x (%#4x:%#4x)\n", 6 - clockCountDown, clockCountDown,cpu.genAddress(cpu.cs,cpu.ip),cpu.cs,cpu.ip);
		clockCountDown -= 1;
		return;
	}
	
	if (cpu.biu.instructionBufferQueuePos < 5)
	{
		const unsigned int address = cpu.genAddress(cpu.cs, cpu.ip);
		cpu.biu.instructionBufferQueue[cpu.biu.instructionBufferQueuePos] = ram.read(address);
		cpu.ip += 1;
		cpu.biu.instructionBufferQueuePos += 1;
		printf("BIU: INSTRUCTION BUFFER QUEUE: queue size %d\n", cpu.biu.instructionBufferQueuePos);

		printf("Instruction buffer: ");
		for (uint8_t b: cpu.biu.instructionBufferQueue)
			printf("%#x ",b);
		putchar('\n');
	}
	
	if (cpu.biu.EUDataAccessClockCountDown > 0)
		cpu.biu.clock = EUDataAccessClock;
	
	clockCountDown = 5;
}

BIU::BIU(): clock(instructionFetchClock), EUDataAccessClockCountDown(0) {}

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
{ return (EURequestReadWord(address+1) << 8) | EURequestINByte(address); }

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
