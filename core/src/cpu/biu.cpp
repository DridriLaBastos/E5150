#include "biu.hpp"
#include "arch.hpp"

using namespace E5150::I8086;

static BIU::InternalInfos BIUWorkingState;

static bool BIUWaitEndOfInterruptDataSaveSequenceClock(void)
{ return !BIUWorkingState.CPU_EXECUTING_INTERRUPT_DATASAVE_SEQUENCE; }

static bool BIUDataAccessClock(void)
{
	BIUWorkingState.EU_DATA_ACCESS_CLOCK_LEFT -= 1;
	return BIUWorkingState.EU_DATA_ACCESS_CLOCK_LEFT == 0;
}

static bool BIUWaitPlaceInInstrutionBufferQueueClock(void)
{ return true; }

static bool BIUInstructionFetchClock(void)
{
	BIUWorkingState.BUS_CYCLE_CLOCK_LEFT -= 1;

	if (BIUWorkingState.BUS_CYCLE_CLOCK_LEFT == 0)
	{
		if (cpu.biu.instructionBufferQueuePos < 5)
		{
			const unsigned int fecthAddress = cpu.genAddress(cpu.cs, cpu.ip + BIUWorkingState.IP_OFFSET);
			const uint8_t instructionByte = ram.read(fecthAddress);
			cpu.biu.instructionBufferQueue[cpu.biu.instructionBufferQueuePos] = instructionByte;
			cpu.biu.instructionBufferQueuePos += 1;
			BIUWorkingState.IP_OFFSET += 1;

			BIUWorkingState.BUS_CYCLE_CLOCK_LEFT = BIUWorkingState.BUS_CYCLE_CLOCK;
			return true;
		}
	}
	return false;
}

const BIU::InternalInfos& BIU::getDebugWorkingState (void)
{ return BIUWorkingState; }

void BIU::clock()
{
	switch (BIUWorkingState.workingMode)
	{
	case BIU::WORKING_MODE::FETCH_INSTRUCTION:
		BIUWorkingState.UPDATE_WORKING_MODE = BIUInstructionFetchClock(); return;
	
	case BIU::WORKING_MODE::FETCH_DATA:
		BIUWorkingState.UPDATE_WORKING_MODE = BIUDataAccessClock(); return;
	
	case BIU::WORKING_MODE::WAIT_ROOM_IN_QUEUE:
		BIUWorkingState.UPDATE_WORKING_MODE = BIUWaitPlaceInInstrutionBufferQueueClock(); return;
	
	case BIU::WORKING_MODE::WAIT_END_OF_INTERRUPT_DATA_SAVE_SEQUENCE:
		BIUWorkingState.UPDATE_WORKING_MODE = BIUWaitEndOfInterruptDataSaveSequenceClock(); return;
	}
}

void BIU::updateClockFunction()
{
	//If a new bus cycle is about to start a new clock function is used to execute custom functions
	if (BIUWorkingState.UPDATE_WORKING_MODE)
	{
		BIUWorkingState.UPDATE_WORKING_MODE = false;
		BIUWorkingState.workingMode = BIU::WORKING_MODE::FETCH_INSTRUCTION;

		//No special action are required but the buffer queue is full
		if (instructionBufferQueuePos >= 5)
			BIUWorkingState.workingMode = BIU::WORKING_MODE::WAIT_ROOM_IN_QUEUE;

		//Biger priority: the EU request data from memory
		if (BIUWorkingState.EU_DATA_ACCESS_CLOCK_LEFT > 0)
			BIUWorkingState.workingMode = BIU::WORKING_MODE::FETCH_DATA;
		
		if (BIUWorkingState.CPU_EXECUTING_INTERRUPT_DATASAVE_SEQUENCE)
			BIUWorkingState.workingMode = BIU::WORKING_MODE::WAIT_END_OF_INTERRUPT_DATA_SAVE_SEQUENCE;
	}
}

void BIU::startInterruptDataSaveSequence() { BIUWorkingState.CPU_EXECUTING_INTERRUPT_DATASAVE_SEQUENCE = true; }
void BIU::endInterruptDataSaveSequence()
{
	instructionBufferQueuePos = 0;
	BIUWorkingState.CPU_EXECUTING_INTERRUPT_DATASAVE_SEQUENCE = false;
	BIUWorkingState.UPDATE_WORKING_MODE = true;
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
		BIUWorkingState.IP_OFFSET = 0;
	}
	BIUWorkingState.UPDATE_WORKING_MODE = true;
}

void BIU::IPToNextInstruction(const unsigned int instructionLength)
{
	cpu.ip += instructionLength;
	BIUWorkingState.IP_OFFSET -= instructionLength;
}

void BIU::instructionBufferQueuePop(const unsigned int n)
{
	for (size_t i = 0; i+n < 5; i++)
		instructionBufferQueue[i] = instructionBufferQueue[i+n];
	instructionBufferQueuePos -= n;
}

void BIU::requestMemoryByte(const unsigned int nBytes) noexcept { BIUWorkingState.EU_DATA_ACCESS_CLOCK_LEFT += nBytes * BIUWorkingState.BUS_CYCLE_CLOCK; }

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
