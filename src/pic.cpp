#include "pic.hpp"

unsigned int log2_2 (const unsigned int n)
{
	ASSERT(n > 0);
	return (n == 1) ? 0 : (1 + log2_2(n-1));
}

E5150::PIC::PIC(PORTS& ports, CPU& connectedCPU): Component("PIC",0b1), m_connectedCPU(connectedCPU), m_initStatus(INIT_STATUS::UNINITIALIZED)
{
	PortInfos commandInfo;
	commandInfo.portNum = 0x20;
	commandInfo.component = this;

	PortInfos dataInfo;
	dataInfo.portNum = 0x21;
	dataInfo.component = this;

	ports.connect(commandInfo);   ports.connect(dataInfo);

	reInit();
}

void E5150::PIC::rotatePriorities (const unsigned int pivot)
{
	assert(pivot < m_priorities.size());
	m_IRLineWithPriority0 = pivot;
	size_t pos = pivot;

	for (unsigned int priority = 0; priority < m_priorities.size(); ++priority)
	{
		m_priorities[pos++] = priority;
		pos %= m_priorities.size();
	}
}

void E5150::PIC::reInit()
{
	m_regs[IMR] = 0;
	//From intel manuel: after initialization of the PIC, the mode is fully nested: the priorities are from 0 to 7
	//with IR0 the highest (0) and IR7 the smallest (7). We rotate the priority with 0 as the pivot
	//which will gives the expected result
	m_picInfo.specialFullyNestedMode = false;
	rotatePriorities(0);
	m_picInfo.slaveID = 7;
	m_nextRegisterToRead = IRR;
}

uint8_t E5150::PIC::readA0_0 () const { return m_regs[m_nextRegisterToRead]; }
uint8_t E5150::PIC::readA0_1 () const { return m_regs[IMR]; }

uint8_t E5150::PIC::read(const unsigned int localAddress)
{ return localAddress ? readA0_1() : readA0_0(); }

static bool isICW1 (const uint8_t icw) { return icw & 0b10000; }
static bool isOCW3 (const uint8_t ocw) { return ocw & 0b1000; }

void E5150::PIC::writeA0_0 (const uint8_t data)
{
	if (isICW1(data))
	{
		m_initStatus = INIT_STATUS::UNINITIALIZED;
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

void E5150::PIC::writeA0_1 (const uint8_t data)
{
	DEBUG("Write to the PIC with A0 = 1");
	if (m_initStatus != INIT_STATUS::INITIALIZED)
		handleInitSequence(data);
	else
		handleOCW1(data);
}

void E5150::PIC::write(const unsigned int localAddress, const uint8_t data)
{
	if (localAddress == 0)
		writeA0_0(data);
	else
		writeA0_1(data);
}

static unsigned int genInterruptVectorForIRLine (const unsigned int T7_T3, const unsigned int IRLineNumber)
{ return (T7_T3 << 3) | IRLineNumber; }

//TODO: test this
void E5150::PIC::interruptInFullyNestedMode (const unsigned int IRLineNumber)
{
	const unsigned int assertedIRLinePriority = m_priorities[IRLineNumber];
	bool canInterrupt = true;

	//First we verify that no interrupt with higher priority is set
	for (size_t i = m_IRLineWithPriority0; m_priorities[i] < assertedIRLinePriority; ++i)
		if (m_regs[ISR] & (1 << i)) canInterrupt = false;

	if (canInterrupt)
	{
		m_connectedCPU.request_intr(genInterruptVectorForIRLine(m_picInfo.T7_T3, IRLineNumber));
		if (!m_picInfo.autoEOI)
			m_regs[ISR] |= (1 << IRLineNumber);
	}
}

void E5150::PIC::assertInteruptLine(const E5150::PIC::INTERRUPT_LINE irLine)
{
	//If the pic is fully initialized
	if (m_initStatus == INIT_STATUS::INITIALIZED)
	{
		//And the interrupt is not masked
		if (m_regs[IMR] & irLine)
		{
			if (!m_picInfo.specialFullyNestedMode)
				throw std::runtime_error("E5150::PIC::assertInteruptLine(): other mode than fully nested not implemented");

			interruptInFullyNestedMode(log2_2(irLine));
		}
	}
}

static bool isICW4Needed (const uint8_t icw1) { return icw1 & 0b1; }
static bool isInSingleMode (const uint8_t icw1) { return icw1 & 0b10; }
static bool isAddressCallIntervalOf4 (const uint8_t icw1) { return icw1 & 0b100; }
static bool isInLevelTriggeredMode (const uint8_t icw1) { return icw1 & 0b1000; }

void E5150::PIC::handleICW1 (const uint8_t icw1)
{
	//TODO: remove this
	if (!isICW4Needed(icw1))
		throw std::logic_error("ICW1: ICW4 is needed to put the PIC in 8086/8088 mode because the IBM PC use an intel 8088");
	m_picInfo.icw4Needed = true;
	m_picInfo.singleMode = isInSingleMode(icw1);
	m_picInfo.addressInterval4 = isAddressCallIntervalOf4(icw1);
	m_picInfo.levelTriggered = isInLevelTriggeredMode(icw1);
	reInit();
}

static unsigned int extractT7_T3 (const uint8_t icw2) { return (icw2 & (~0b111)) >> 3; }

void E5150::PIC::handleICW2 (const uint8_t icw2)
{ m_picInfo.T7_T3 = extractT7_T3(icw2); }

void E5150::PIC::handleICW3 (const uint8_t icw3)
{ /*TODO: what to put here ?*/ }

static bool isInMode8086_8088 (const uint8_t icw4) { return icw4 & 0b1; }
static bool isAutoEOI (const uint8_t icw4) { return icw4 & 0b10; }
static bool isMaster (const uint8_t icw4) { return icw4 & 0b100; }
static bool isInBufferedMode (const uint8_t icw4) { return icw4 & 0b1000; }
static bool isInSpecialFullyNestedMode (const uint8_t icw4) { return icw4 & 0b10000; }

void E5150::PIC::handleICW4 (const uint8_t icw4)
{
	if (!isInMode8086_8088(icw4))
		throw std::logic_error("The PIC can't be in MCS-80-85 mode in an IBM PC");
	
	m_picInfo.autoEOI= isAutoEOI(icw4);

	if (isInBufferedMode(icw4))
		m_picInfo.bufferedMode = isMaster(icw4) ? BUFFERED_MODE::MASTER : BUFFERED_MODE::SLAVE;
	else
		m_picInfo.bufferedMode = BUFFERED_MODE::NON;
	
	m_picInfo.specialFullyNestedMode = isInSpecialFullyNestedMode(icw4);
}

void E5150::PIC::handleInitSequence (const uint8_t icw)
{
	switch (m_initStatus)
	{
		//Wether the pic is initialized or not, if it receive an icw1, it restart its initialization sequence
		case INIT_STATUS::UNINITIALIZED:
		case INIT_STATUS::INITIALIZED:
			handleICW1(icw);
			m_initStatus = INIT_STATUS::ICW2;
			break;
		
		case INIT_STATUS::ICW2:
		{
			handleICW2(icw);

			if (m_picInfo.singleMode)
				m_initStatus = m_picInfo.icw4Needed ? INIT_STATUS::ICW4 : INIT_STATUS::INITIALIZED;
			else
				m_initStatus = INIT_STATUS::ICW4;

		} break;
		
		case INIT_STATUS::ICW3:
			handleICW3(icw);
			m_initStatus = m_picInfo.icw4Needed ? INIT_STATUS::ICW4 : INIT_STATUS::INITIALIZED;
			break;
		
		case INIT_STATUS::ICW4:
			handleICW4(icw);
			m_initStatus = INIT_STATUS::INITIALIZED;
			break;
	}
}

void E5150::PIC::handleOCW1 (const uint8_t ocw1)
{ m_regs[IMR] = ocw1; }

void E5150::PIC::nonSpecificEOI()
{
	unsigned int ISBitToReset;

	for (size_t i = m_IRLineWithPriority0; m_priorities[i] <= 7; ++i)
	{
		if (m_regs[ISR] & (1 << i))
			m_regs[ISR] &= ~(1 << i);
	}
}

void E5150::PIC::specificEOI(const unsigned int IRLevelToBeActedUpon)
{
	unsigned int ISRBitsToClear = 0;

	for (size_t i = 0; i < m_priorities.size(); ++i)
	{
		if (m_priorities[i] == IRLevelToBeActedUpon)
			ISRBitsToClear |= (1 << i);
	}
	m_regs[ISR] &= ~(ISRBitsToClear);
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
void E5150::PIC::handleOCW2 (const uint8_t ocw2)
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

void E5150::PIC::handleOCW3 (const uint8_t ocw3)
{
	if (isRRSet(ocw3))
		m_nextRegisterToRead = isRISSet(ocw3) ? ISR : IRR;
	//TODO: implement
}