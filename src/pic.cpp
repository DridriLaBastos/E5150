#include "pic.hpp"

#define PICDebug(REQUIRED_DEBUG_LEVEL,...) debug<REQUIRED_DEBUG_LEVEL>("PIC: " __VA_ARGS__)

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

E5150::PIC::PIC(PORTS& ports, CPU& connectedCPU): Component("PIC",ports,0x20,0b1), connectedCPU(connectedCPU), initStatus(INIT_STATUS::UNINITIALIZED)
{
	pic = this;
	regs[ISR]=0;
	reInit();
}

static uint8_t readA0_0 (void) { return pic->regs[pic->nextRegisterToRead]; }
static uint8_t readA0_1 (void) { return pic->regs[PIC::IMR]; }

uint8_t E5150::PIC::read(const unsigned int localAddress)
{ pic = this; return localAddress ? readA0_1() : readA0_0(); }

//TODO: not clearing the IS bit in special mask mode
//TODO: investigate the 'last level aknwoledged' sentence
static void nonSpecificEOI(void)
{
	unsigned int highestPriorityIRLineFound = 7;
	bool inServiceInterruptFound = false;

	for (size_t i = 0; i < pic->priorities.size(); ++i)
	{
		//If the interrupt has been serviced
		if ((1 << i) & pic->regs[PIC::ISR])
		{
			inServiceInterruptFound = true;
			if (pic->priorities[i] < pic->priorities[highestPriorityIRLineFound])
				highestPriorityIRLineFound = i;
		}
	}

	if (inServiceInterruptFound)
		pic->regs[PIC::ISR] &= ~(1 << highestPriorityIRLineFound);
}

static void specificEOI(const unsigned int priorityLevel)
{
	unsigned int ISRBitsToClear = 0;

	for (size_t i = 0; i < pic->priorities.size(); ++i)
	{
		if ((1 << i) & pic->regs[PIC::ISR])
		{
			if (pic->priorities[i] == priorityLevel)
			{
				pic->regs[PIC::ISR] &= ~(1 << i);
				std::cout << "reseting ir line " << i << "\n";
				break;
			}
		}
	}
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
	const unsigned int D765 = ocw2 >> 5;

	switch (D765)
	{
	case 0b010://no op
		break;

	case 0b1:
		nonSpecificEOI();
		break;
	
	case 0b11:
		specificEOI(getIRLevel(ocw2));
		break;

	default:
		PICDebug(1,"Only no operation, and non-specific EOI are implemented");
		break;
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
static bool isInLevelTriggeredMode (const uint8_t icw1) { return icw1 & 0b1000; }
static void handleICW1(const uint8_t icw1)
{
	//TODO: remove this
	if (!isICW4Needed(icw1))
		throw std::logic_error("ICW1: ICW4 is needed to put the PIC in 8086/8088 mode because the IBM PC use an intel 8088");
	pic->info.icw4Needed = true;
	pic->info.singleMode = isInSingleMode(icw1);
	pic->info.levelTriggered = isInLevelTriggeredMode(icw1);

	PICDebug(7,"ICW1: ICW4 needed | {} mode | {} triggered mode", (pic->info.singleMode ? "single" : "cascade"), (pic->info.levelTriggered ? "level" : "edge"));

	reInit();
}

static unsigned int extractFirstInterruptVectorFromICW2 (const uint8_t icw2) { return icw2 & (~0b111); }
static void handleICW2(const uint8_t icw2)
{
	pic->info.firstInterruptVector = extractFirstInterruptVectorFromICW2(icw2);
	PICDebug(7,"ICW2: interrupt vector from {:#x} to {:#x}",pic->info.firstInterruptVector,pic->info.firstInterruptVector+7);
}

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
	PICDebug(7,"ICW4: 8086/8088 mode | {} eoi | {} | {}special fully nested mode",pic->info.autoEOI ? "auto" : "normal", "no description available", (pic->info.specialFullyNestedMode) ? "" : "not ");
}

static void handleInitSequence(const uint8_t icw)
{
	switch (pic->initStatus)
	{
		//Wether the pic is initialized or not, if it receive an icw1, it restart its initialization sequence
		case PIC::INIT_STATUS::UNINITIALIZED:
		case PIC::INIT_STATUS::INITIALIZED:
			PICDebug(7,"Starting initialization sequence");
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
			PICDebug(7,"Fully initialized");
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
		if (pic->initStatus == PIC::INIT_STATUS::INITIALIZED)
		{
			if (isOCW3(data))
				handleOCW3(data);
			else
				handleOCW2(data);
		}
		else
			PICDebug(5,"Not initialized, OCW data not writen");
	}
}

static void handleOCW1 (const uint8_t ocw1)
{
	pic->regs[PIC::IMR] = ocw1;
	PICDebug(7,"OCW1: Interrupt mask: {:b}",ocw1);
}

static void writeA0_1 (uint8_t data)
{
	if (pic->initStatus != PIC::INIT_STATUS::INITIALIZED)
		handleInitSequence(data);
	else
	{
		if (pic->initStatus == PIC::INIT_STATUS::INITIALIZED)
			handleOCW1(data);
		else
			PICDebug(5,"Not initialized, OCW data not writen");
	}
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
			throw std::runtime_error("PIC write to invalid address " + std::to_string(localAddress));
			break;
	}
}

static unsigned int genInterruptVectorForIRLine (const unsigned int IRNumber)
{ return pic->info.firstInterruptVector + IRNumber; }

//TODO: test this
static void interruptInFullyNestedMode (const unsigned int IRNumber)
{

	const unsigned int assertedIRLinePriority = pic->priorities[IRNumber];
	bool canInterrupt = true;

	//First we check that no interrupt with higher priority is set
	for (size_t i = pic->IRLineWithPriority0; pic->priorities[i] < assertedIRLinePriority; ++i)
	{
		if (pic->regs[PIC::ISR] & (1 << i))
		{
			PICDebug(DEBUG_LEVEL_MAX,"IR {} with priority {} masks IR {} with priority {}",i,pic->priorities[i],IRNumber,pic->priorities[IRNumber]);
			return;
		}
	}

	const unsigned int interruptVectorGenerated = genInterruptVectorForIRLine(IRNumber);
	PICDebug(10,"Generating interrupt vector {:#x} for line {}",interruptVectorGenerated,IRNumber);
	pic->connectedCPU.request_intr(interruptVectorGenerated);
	if (!pic->info.autoEOI)
		pic->regs[PIC::ISR] |= (1 << IRNumber);
}

//TODO: clock accurate interrupt generation
void E5150::PIC::assertInterruptLine(const E5150::PIC::IR_LINE IRLine,const Component* caller)
{
	if (caller != nullptr)
		PICDebug(DEBUG_LEVEL_MAX,"Interrupt request on line {} for {}",IRLine,caller->m_name);
	pic = this;
	//If the pic is fully initialized
	if (initStatus == INIT_STATUS::INITIALIZED)
	{
		//If the interrupt isn't masked
		if ((1 << IRLine) & (~pic->regs[IMR]))
		{
			if (info.specialFullyNestedMode)
				throw std::runtime_error("E5150::PIC::assertInterruptLine(): other mode than fully nested not implemented");

			interruptInFullyNestedMode(IRLine);
		}
		else
			PICDebug(DEBUG_LEVEL_MAX,"Interrupt request on line {} masked (IMR: {:b})",IRLine,regs[IMR]);
	}
	else
		PICDebug(8, "Interrupt request (on line {}) while FDC is not initialized does nothing", IRLine);
}
