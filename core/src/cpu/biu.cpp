#include "biu.hpp"
#include "arch.hpp"

using namespace E5150::I8086;

enum class BIU_WORKING_MODE 
{
	FETCH_INSTRUCTION,
	FETCH_DATA,
	WAIT_ROOM_IN_QUEUE,
	WAIT_END_OF_INTERRUPT_DATA_SAVE_SEQUENCE
};

//Some status variables are created here because they are related to the internal working of the BIU emulation and thus they don't need to be visible in the header file of the class
static constexpr unsigned int BUS_CYCLE_CLOCK = 4;
static unsigned int BUS_CYCLE_CLOCK_LEFT = BUS_CYCLE_CLOCK;
static unsigned int EU_DATA_ACCESS_CLOCK_LEFT = 0;
static bool CPU_EXECUTING_INTERRUPT_DATASAVE_SEQUENCE = false;
static bool UPDATE_WORKING_MODE = false;
static BIU_WORKING_MODE workingMode = BIU_WORKING_MODE::FETCH_INSTRUCTION;
static unsigned int IP_OFFSET = 0;

static bool BIUWaitEndOfInterruptDataSaveSequenceClock(void)
{ return !CPU_EXECUTING_INTERRUPT_DATASAVE_SEQUENCE; }

static bool BIUDataAccessClock(void)
{
	EU_DATA_ACCESS_CLOCK_LEFT -= 1;
	return EU_DATA_ACCESS_CLOCK_LEFT == 0;
}

static bool BIUWaitPlaceInInstrutionBufferQueueClock(void)
{ return true; }

static bool BIUInstructionFetchClock(void)
{
	BUS_CYCLE_CLOCK_LEFT -= 1;

	if (BUS_CYCLE_CLOCK_LEFT == 0)
	{
		if (cpu.biu.instructionBufferQueuePos < 5)
		{
			const unsigned int fecthAddress = cpu.genAddress(cpu.cs, cpu.ip + IP_OFFSET);
			const uint8_t instructionByte = ram.read(fecthAddress);
			cpu.biu.instructionBufferQueue[cpu.biu.instructionBufferQueuePos] = instructionByte;
			cpu.biu.instructionBufferQueuePos += 1;
			IP_OFFSET += 1;

			BUS_CYCLE_CLOCK_LEFT = BUS_CYCLE_CLOCK;
			return true;
		}
	}
	return false;
}

void BIU::debugClockPrint()
{
	switch (workingMode)
	{
		case BIU_WORKING_MODE::FETCH_INSTRUCTION:
		{
			printf("BIU: BUS CYCLE %d (clock count down: %d) --- FETCHING %#5x (%#4x:%#4x)\n", BUS_CYCLE_CLOCK - BUS_CYCLE_CLOCK_LEFT, BUS_CYCLE_CLOCK_LEFT,cpu.genAddress(cpu.cs,cpu.ip+IP_OFFSET),cpu.cs,cpu.ip+IP_OFFSET);

			if ((cpu.biu.instructionBufferQueuePos <= 5) && BUS_CYCLE_CLOCK_LEFT == BUS_CYCLE_CLOCK)
			{
				printf("BIU: INSTRUCTION BUFFER QUEUE: queue size %d\n", cpu.biu.instructionBufferQueuePos);

				printf("Instruction buffer: ");
				std::for_each(cpu.biu.instructionBufferQueue.begin(), cpu.biu.instructionBufferQueue.end(),
					[](const uint8_t b) { printf("%#x ",b); });
				// for (uint8_t b: cpu.biu.instructionBufferQueue)
				// 	printf("%#x ",b);
				putchar('\n');
			}
		} break;

		case BIU_WORKING_MODE::FETCH_DATA:
			printf("BIU: DATA ACCESS FROM EU: clock left: %d\n", EU_DATA_ACCESS_CLOCK_LEFT);
			break;
		
		case BIU_WORKING_MODE::WAIT_ROOM_IN_QUEUE:
			printf("BIU: INSTRUCTION BUFFER QUEUE FULL\n");
			break;

		case BIU_WORKING_MODE::WAIT_END_OF_INTERRUPT_DATA_SAVE_SEQUENCE:
			printf("BIU: WAITING END OF INTERRUPT DATA SAVING PROCEDURE\n");
			break;

	default:
		break;
	}
}

void BIU::clock()
{
	switch (workingMode)
	{
	case BIU_WORKING_MODE::FETCH_INSTRUCTION:
		UPDATE_WORKING_MODE = BIUInstructionFetchClock(); return;
	
	case BIU_WORKING_MODE::FETCH_DATA:
		UPDATE_WORKING_MODE = BIUDataAccessClock(); return;
	
	case BIU_WORKING_MODE::WAIT_ROOM_IN_QUEUE:
		UPDATE_WORKING_MODE = BIUWaitPlaceInInstrutionBufferQueueClock(); return;
	
	case BIU_WORKING_MODE::WAIT_END_OF_INTERRUPT_DATA_SAVE_SEQUENCE:
		UPDATE_WORKING_MODE = BIUWaitEndOfInterruptDataSaveSequenceClock(); return;
	}
}

void BIU::debug(void)
{
	std::for_each(instructionBufferQueue.begin(), instructionBufferQueue.end(), [](const uint8_t& b)
				  { printf("%#4x ",b); });
	putchar('\n');
	for (int i = 0; i < instructionBufferQueuePos; ++i)
	{ printf("     "); }
	puts("  ^");
}

void BIU::updateClockFunction()
{
	//If a new bus cycle is about to start a new clock function is used to execute custom functions
	if (UPDATE_WORKING_MODE)
	{
		UPDATE_WORKING_MODE = false;
		workingMode = BIU_WORKING_MODE::FETCH_INSTRUCTION;

		//No special action are required but the buffer queue is full
		if (instructionBufferQueuePos >= 5)
			workingMode = BIU_WORKING_MODE::WAIT_ROOM_IN_QUEUE;

		//Biger priority: the EU request data from memory
		if (EU_DATA_ACCESS_CLOCK_LEFT > 0)
			workingMode = BIU_WORKING_MODE::FETCH_DATA;
		
		if (CPU_EXECUTING_INTERRUPT_DATASAVE_SEQUENCE)
			workingMode = BIU_WORKING_MODE::WAIT_END_OF_INTERRUPT_DATA_SAVE_SEQUENCE;
	}
}

void BIU::startInterruptDataSaveSequence() { CPU_EXECUTING_INTERRUPT_DATASAVE_SEQUENCE = true; }
void BIU::endInterruptDataSaveSequence()
{
	instructionBufferQueuePos = 0;
	CPU_EXECUTING_INTERRUPT_DATASAVE_SEQUENCE = false;
	UPDATE_WORKING_MODE = true;
}

/**
 * @brief Reset the instruction buffer queue and start a new instruction stream if a jump happened
 * 
 * @details This function must be called after a jump (or a successfull conditionnal jump). It has two effects:
 * 
 * - The first as mentionned is to reset the instruction queue. The queue will be cleared of all bytes
 * - The second is to make the cpu fetch data from IP again. Because of the design of 8086/8088 the currently fetched instruction address may be ahead from the address in cs:ip. After a jump, the program flow changed
 * 
 * @param didJump For conditionnal jump, specidfy if the jump happened or not
 */
void BIU::endControlTransferInstruction (const bool didJump)
{
	//At the end of the execution <instructionLength> octets are poped from the instruction buffer queue.
	//When flushing after a jmp, the length is set to 5 (the size of the queue of the 8088), this make the buffer queue to be popped
	//The position in the queue is also set to 5 because in the implementaion, the amount of elements to pop is substracted to the position in the queue. If there is 4 (or less) elements in the queue when poping, the new pos is 4 - 5 = -1 = 2^32 in unsigned and the queu will be always considered as full and no new instructions will be fetched
	//This parameter is only usefull for conditionnal jmp. If the jmp didn't happens, only the <instructionLength> octet have to be removed from the queue (the 8086::clock function takes care of that)
	if (didJump)
	{
		cpu.eu.instructionLength  = 5;
		instructionBufferQueuePos = 5;
		IP_OFFSET = 0;
	}
	UPDATE_WORKING_MODE = true;
}

void BIU::IPToNextInstruction(const unsigned int instructionLength)
{
	cpu.ip += instructionLength;
	IP_OFFSET -= instructionLength;
}

void BIU::instructionBufferQueuePop(const unsigned int n)
{
	for (size_t i = 0; i+n < 5; i++)
		instructionBufferQueue[i] = instructionBufferQueue[i+n];
	instructionBufferQueuePos -= n;
}

void BIU::requestMemoryByte(const unsigned int nBytes) noexcept { EU_DATA_ACCESS_CLOCK_LEFT += nBytes * BUS_CYCLE_CLOCK; }

uint8_t BIU::readByte (const unsigned int address) const
{
	Arch::_addressBus = address;
	return ram.read(address);
}
uint16_t BIU::readWord (const unsigned int address) const { return (readByte(address + 1) << 8) | readByte(address); }

void BIU::writeByte (const unsigned int address, const uint8_t byte) const
{
	ram.write(address, byte);
	Arch::_addressBus = address;
	Arch::_dataBus    = byte;
}
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
