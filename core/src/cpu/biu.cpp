#include "biu.hpp"
#include "arch.hpp"

using namespace E5150::I8086;

BIU::BIU(): mClockCountDown(5) {}

void BIU::clock()
{
	printf("%#x (%#x, %#x)\n", (unsigned)addressBus, cpu.cs, cpu.ip);
	if (mClockCountDown > 0)
	{
		printf("BIU: BUS CYCLE %d (clock count down: %d)\n", 6 - mClockCountDown, mClockCountDown);
		mClockCountDown -= 1;
		return;
	}

	if (instructionBufferQueuePos < 5)
	{
		ram.read();
		instructionBufferQueue[instructionBufferQueuePos] = dataBus;
		cpu.ip += 1;
		instructionBufferQueuePos += 1;
		addressBus = cpu.genAddress(cpu.cs,cpu.ip);
		printf("BIU: INSTRUCTION BUFFER QUEUE: queue size %d\n", instructionBufferQueuePos);

		printf("Instruction buffer: ");
		for (uint8_t b: instructionBufferQueue)
			printf("%#x ",b);
		putchar('\n');
	}

	mClockCountDown += 5;
}

void BIU::instructionBufferQueuePop(const unsigned int n)
{
	for (size_t i = 0; i+n < 5; i++)
		instructionBufferQueue[i] = instructionBufferQueue[i+n];
	instructionBufferQueuePos -= n;
}

uint8_t readByte (const unsigned int address);
uint16_t readWord (const unsigned int address);
void writeByte(const unsigned int address, const uint8_t data);
void writeWord(const unsigned int address, const uint16_t data);

uint8_t inByte (const unsigned int address);
uint16_t inWord (const unsigned int address);
void outByte(const unsigned int address, const uint8_t data);
void outWord(const unsigned int address, const uint16_t data);
