#include "pic.hpp"

using namespace E5150;
static PIC* pic = nullptr;

static void rotatePriorities (const unsigned int pivot)
{
	assert(pivot < pic->priorities.size());
	pic->IRLineWithPriority0 = pivot;
	size_t pos = pivot;

	for (unsigned int priority = 0; priority < pic->priorities.size(); ++priority)
	{
		pic->priorities[pos++] = priority;
		pos %= pic->priorities.size();
	}
}

static void reInit(void)
{
	pic->regs[E5150::PIC::IMR] = 0;
	//From intel manuel: after initialization of the PIC, the mode is fully nested: the priorities are from 0 to 7
	//with IR0 the highest (0) and IR7 the smallest (7). We rotate the priority with 0 as the pivot
	//which will gives the expected result
	pic->info.specialFullyNestedMode = false;
	rotatePriorities(0);
	pic->info.slaveID = 7;
	pic->nextRegisterToRead = PIC::REGISTER::IRR;
}

E5150::PIC::PIC(PORTS& ports, CPU& connectedCPU): Component("PIC",ports,0x20,0b1), m_connectedCPU(connectedCPU), initStatus(INIT_STATUS::UNINITIALIZED)
{
	pic = this;
	reInit();
}

static uint8_t readA0_0 (void) { return pic->regs[pic->nextRegisterToRead]; }
static uint8_t readA0_1 (void) { return pic->regs[PIC::IMR]; }

uint8_t E5150::PIC::read(const unsigned int localAddress)
{ pic = this; return localAddress ? readA0_1() : readA0_0(); }

static void nonSpecificEOI(void)
{
	unsigned int ISBitToReset;

	for (size_t i = pic->IRLineWithPriority0; pic->priorities[i] <= 7; ++i)
	{
		if (pic->regs[PIC::ISR] & (1 << i))
			pic->regs[PIC::ISR] &= ~(1 << i);
	}
}

static void specificEOI(const unsigned int IRLevelToBeActedUpon)
{
	unsigned int ISRBitsToClear = 0;

	for (size_t i = 0; i < pic->priorities.size(); ++i)
	{
		if (pic->priorities[i] == IRLevelToBeActedUpon)
			ISRBitsToClear |= (1 << i);
	}
	pic->regs[PIC::ISR] &= ~(ISRBitsToClear);
}

static bool isRSet   (const uint8_t ocw2) { return ocw2 & (1 << 7); }
static bool isSLSet  (const uint8_t ocw2) { return ocw2 & (1 << 6); }
static bool isEOISet (const uint8_t ocw2) { return ocw2 & (1 << 5); }
static unsigned int getIRLevel (const uint8_t ocw2) { return ocw2 & 0b111; }
/**
 * From intel manual for the 8259A:
 *          7   6   5   4   3   2   1   0
 * A0 = 0 | R | SL|EOI| 0 | 0 | L2| L1| L0| 
 * 
 *	  | R | SL|EOI|
 *	  | 0 | 0 | 0 |	non specific EOI command				+--> end of interrupt
 *	  | 0 | 1 | 1 |	specific EOI command					+
 *	  | 1 | 0 | 1 |	rotate on non specific EOI command		+--> automatic rotation
 *	  | 1 | 0 | 0 |	rotate in automatic EOI mode (set)		|
 *	  | 0 | 0 | 0 |	rotate in automatic EOI mode (clear)	+
 *	  | 1 | 1 | 1 |	rotate on specific EOI command			+--> specific rotation
 *	  | 1 | 1 | 0 |	set priority command					+
 *	  | 0 | 1 | 0 |	no operation
 */
static void handleOCW2 (const uint8_t ocw2)
{
	const unsigned int IRLevelToBeActedUpon = getIRLevel(ocw2);
	if (isRSet(ocw2))
	{
		throw std::runtime_error("OCW2: R not implemented");
	}
	else
	{
		if (isEOISet(ocw2))
		{
			if (isSLSet(ocw2))
				specificEOI(IRLevelToBeActedUpon);
			else
				nonSpecificEOI();
		}
		else
		{
			throw std::runtime_error("OCW2: r eoi not implemented");
		}
	}
}

static bool isRISSet (const uint8_t ocw3) { return ocw3 & 0b1; }
static bool isRRSet (const uint8_t ocw3) { return ocw3 & 0b10; }

static void handleOCW3 (const uint8_t ocw3)
{
	if (isRRSet(ocw3))
		pic->nextRegisterToRead = isRISSet(ocw3) ? PIC::ISR : PIC::IRR;
	//TODO: implement
}

static bool isICW4Needed (const uint8_t icw1) { return icw1 & 0b1; }
static bool isInSingleMode (const uint8_t icw1) { return icw1 & 0b10; }
static bool isAddressCallIntervalOf4 (const uint8_t icw1) { return icw1 & 0b100; }
static bool isInLevelTriggeredMode (const uint8_t icw1) { return icw1 & 0b1000; }
static void handleICW1(const uint8_t icw1)
{
	//TODO: remove this
	if (!isICW4Needed(icw1))
		throw std::logic_error("ICW1: ICW4 is needed to put the PIC in 8086/8088 mode because the IBM PC use an intel 8088");
	pic->info.icw4Needed = true;
	pic->info.singleMode = isInSingleMode(icw1);
	pic->info.addressInterval4 = isAddressCallIntervalOf4(icw1);
	pic->info.levelTriggered = isInLevelTriggeredMode(icw1);
	reInit();
}

static unsigned int extractT7_T3FromICW2 (const uint8_t icw2) { return (icw2 & (~0b111)) >> 3; }
static void handleICW2(const uint8_t icw2)
{ pic->info.T7_T3 = extractT7_T3FromICW2(icw2); }

static void handleICW3 (const uint8_t icw3)
{ /*TODO: what to put here ?*/ }

static bool isInMode8086_8088 (const uint8_t icw4) { return icw4 & 0b1; }
static bool isAutoEOI (const uint8_t icw4) { return icw4 & 0b10; }
static bool isMaster (const uint8_t icw4) { return icw4 & 0b100; }
static bool isInBufferedMode (const uint8_t icw4) { return icw4 & 0b1000; }
static bool isInSpecialFullyNestedMode (const uint8_t icw4) { return icw4 & 0b10000; }
static void handleICW4 (const uint8_t icw4)
{
	if (!isInMode8086_8088(icw4))
		throw std::logic_error("The PIC can't be in MCS-80-85 mode in an IBM PC");
	
	pic->info.autoEOI= isAutoEOI(icw4);

	if (isInBufferedMode(icw4))
		pic->info.bufferedMode = isMaster(icw4) ? PIC::BUFFERED_MODE::MASTER : PIC::BUFFERED_MODE::SLAVE;
	else
		pic->info.bufferedMode = PIC::BUFFERED_MODE::NON;
	
	pic->info.specialFullyNestedMode = isInSpecialFullyNestedMode(icw4);
}

static void handleInitSequence(const uint8_t icw)
{
	switch (pic->initStatus)
	{
		//Wether the pic is initialized or not, if it receive an icw1, it restart its initialization sequence
		case PIC::INIT_STATUS::UNINITIALIZED:
		case PIC::INIT_STATUS::INITIALIZED:
			handleICW1(icw);
			pic->initStatus = PIC::INIT_STATUS::ICW2;
			break;
		
		case PIC::INIT_STATUS::ICW2:
		{
			handleICW2(icw);

			if (pic->info.singleMode)
				pic->initStatus = pic->info.icw4Needed ? PIC::INIT_STATUS::ICW4 : PIC::INIT_STATUS::INITIALIZED;
			else
				pic->initStatus = PIC::INIT_STATUS::ICW4;

		} break;
		
		case PIC::INIT_STATUS::ICW3:
			handleICW3(icw);
			pic->initStatus = pic->info.icw4Needed ? PIC::INIT_STATUS::ICW4 : PIC::INIT_STATUS::INITIALIZED;
			break;
		
		case PIC::INIT_STATUS::ICW4:
			handleICW4(icw);
			pic->initStatus = PIC::INIT_STATUS::INITIALIZED;
			break;
	}
}

static bool isICW1 (const uint8_t icw) { return icw & 0b10000; }
static bool isOCW3 (const uint8_t ocw) { return ocw & 0b1000; }
static void writeA0_0 (const uint8_t data)
{
	if (isICW1(data))
	{
		pic->initStatus = PIC::INIT_STATUS::UNINITIALIZED;
		handleInitSequence(data);
	}
	else
	{
		if (isOCW3(data))
			handleOCW3(data);
		else
			handleOCW2(data);
	}
}

static void handleOCW1 (const uint8_t ocw1)
{ pic->regs[PIC::IMR] = ocw1; }

static void writeA0_1 (uint8_t data)
{
	DEBUG("Write to the PIC with A0 = 1");
	if (pic->initStatus != PIC::INIT_STATUS::INITIALIZED)
		handleInitSequence(data);
	else
		handleOCW1(data);
}

void E5150::PIC::write(const unsigned int localAddress, const uint8_t data)
{
	pic = this;
	switch (localAddress)
	{
		case 0:
			writeA0_0(data);
			break;
		
		case 1:
			writeA0_1(data);
			break;
		
		default:
			//TODO: error
			break;
	}
}

static unsigned int genInterruptVectorForIRLine (const unsigned int T7_T3, const unsigned int IRLineNumber)
{ return (T7_T3 << 3) | IRLineNumber; }

//TODO: test this
void E5150::PIC::interruptInFullyNestedMode (const unsigned int IRLineNumber)
{
	const unsigned int assertedIRLinePriority = priorities[IRLineNumber];
	bool canInterrupt = true;

	//First we verify that no interrupt with higher priority is set
	for (size_t i = IRLineWithPriority0; priorities[i] < assertedIRLinePriority; ++i)
		if (regs[ISR] & (1 << i)) canInterrupt = false;

	if (canInterrupt)
	{
		m_connectedCPU.request_intr(genInterruptVectorForIRLine(info.T7_T3, IRLineNumber));
		if (!info.autoEOI)
			regs[ISR] |= (1 << IRLineNumber);
	}
}

void E5150::PIC::assertInteruptLine(const E5150::PIC::INTERRUPT_LINE irLine)
{
	//If the pic is fully initialized
	if (initStatus == INIT_STATUS::INITIALIZED)
	{
		//And the interrupt is not masked
		if (regs[IMR] & irLine)
		{
			if (!info.specialFullyNestedMode)
				throw std::runtime_error("E5150::PIC::assertInteruptLine(): other mode than fully nested not implemented");

			interruptInFullyNestedMode(irLine);
		}
	}
}
