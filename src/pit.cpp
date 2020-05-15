#include "pit.hpp"

E5150::PIT::PIT(PORTS& ports, PIC& connectedPIC): Component("PIT",0b11), m_connectedPIC(connectedPIC)
{
	for (Counter& c: m_counters)
	{
		c.isCounting = false;
		c.readComplete = true;
	}

	for (size_t modeIndex = 0; modeIndex < 6; ++modeIndex)
	{
		for (size_t counterIndex = 0; counterIndex < 3; ++counterIndex)
			m_modes[modeIndex][counterIndex] = std::unique_ptr<MODE> (new MODE(m_counters[counterIndex]));
	}
	
	PortInfos info0x40;
	info0x40.component = this;
	info0x40.portNum = 0x40;

	PortInfos info0x41;
	info0x41.component = this;
	info0x41.portNum  = 0x41;

	PortInfos info0x42;
	info0x42.component = this;
	info0x42.portNum = 0x42;

	PortInfos info0x43;
	info0x43.component = this;
	info0x43.portNum = 0x43;

	ports.connect(info0x40);
	ports.connect(info0x41);
	ports.connect(info0x42);
	ports.connect(info0x43);   
}

void E5150::PIT::toggleGateFor(const COUNTER counterNumber)
{
	Counter& counter = m_counters[static_cast<unsigned int>(counterNumber)];
	counter.gateValue = (counter.gateValue == OUTPUT_VALUE::LOW) ? OUTPUT_VALUE::HIGH : OUTPUT_VALUE::LOW;
}

//TODO: implement the one clock wait to confirm writing to the counter
void E5150::PIT::clock()
{
	if (m_counters[0].isCounting)
		clockForCounter0();
	
	if (m_counters[1].isCounting)
		clockForCounter1();
	
	if (m_counters[2].isCounting)
		clockForCounter2();
}

void E5150::PIT::clockForCounter0()
{
	OUTPUT_VALUE oldOutPutValue = m_counters[0].outputValue;
	m_counters[0].mode->clock();

	//Going from low to high
	if ((m_counters[0].outputValue == OUTPUT_VALUE::HIGH) && (oldOutPutValue == OUTPUT_VALUE::LOW))
		m_connectedPIC.assertInteruptLine(PIC::IR0);
}

void E5150::PIT::clockForCounter1() {}
void E5150::PIT::clockForCounter2() {}

void E5150::PIT::writeCounter (const unsigned int counterIndex, const uint8_t data)
{ m_counters[counterIndex].mode->writeOperation(data); }

static bool isM0Set (const uint8_t controlWord) { return controlWord & 0b10; }
static bool isM1Set (const uint8_t controlWord) { return controlWord & 0b100; }
static bool isM2Set (const uint8_t controlWord) { return controlWord & 0b1000; }

void E5150::PIT::setModeForCounter (const unsigned int counterIndex, const uint8_t controlWord)
{
	if (isM1Set(controlWord))
	{
		if (isM0Set(controlWord)) m_modes[3][counterIndex]->enable();
		else m_modes[2][counterIndex]->enable();
	}
	else
	{
		if (isM2Set(controlWord))
		{
			if (isM0Set(controlWord)) m_modes[5][counterIndex]->enable();
			else m_modes[2][counterIndex]->enable();
		}
		else
		{
			if (isM0Set(controlWord)) m_modes[1][counterIndex]->enable();
			else m_modes[0][counterIndex]->enable();
		}
	}
}

static unsigned int extractRegisterNumber (const uint8_t controlWord) { return (controlWord & 0b11000000) >> 6; }

void E5150::PIT::writeControlWord (const uint8_t controlWord)
{
	const unsigned int counterIndex = extractRegisterNumber(controlWord);

	setModeForCounter(counterIndex, controlWord);
	setOperationAccessForCounter(counterIndex, controlWord);
	m_counters[counterIndex].codedBCD = controlWord & 0b1;
}

void E5150::PIT::write (const unsigned int localAddress, const uint8_t data)
{
	if (localAddress == 0b11)
		writeControlWord(data);
	else
		writeCounter(localAddress, data);
}

//TODO: how does the IBM PC protect the counter from input clock while the read od the count value
//isn't finished ?
uint8_t E5150::PIT::applyPICReadAlgorithm (Counter& counter, const unsigned int value)
{
	switch (counter.readStatus)
	{
		case OPERATION_STATUS::LSB:
		{
			if (counter.accessOperation == ACCESS_OPERATION::LSB_MSB)
			{
				counter.readStatus = OPERATION_STATUS::MSB;
				counter.readComplete = false;
			}

			return value & 0xFF;
		}
		
		case OPERATION_STATUS::MSB:
		{
			counter.readStatus = OPERATION_STATUS::LSB;
			counter.readComplete = true;

			if (counter.accessOperation == ACCESS_OPERATION::LSB_MSB)
				counter.readStatus = OPERATION_STATUS::LSB;

			return (value & 0xFF00) >> 8;
		}
	}
}

uint8_t E5150::PIT::readCounterLatchedValue (Counter& counter)
{
	const uint8_t result = applyPICReadAlgorithm(counter, counter.latchedValue);

	if (counter.readComplete)
		counter.latchedValueIsAvailable = false;

	return result;
}

uint8_t E5150::PIT::readCounterDirectValue (Counter& counter)
{ return applyPICReadAlgorithm(counter, counter.counterValue.word); }

uint8_t E5150::PIT::read (const unsigned int localAddress)
{
	uint8_t ret = 0;

	if (localAddress != 0b11)
	{
		Counter& counter = m_counters[localAddress];
		ret = (counter.latchedValueIsAvailable) ? readCounterLatchedValue(counter) : readCounterDirectValue(counter);
	}
	
	return ret;
}

static bool isRL0Set (const uint8_t controlWord) { return controlWord & 0b10000; }
static bool isRL1Set (const uint8_t controlWord) { return controlWord & 0b100000; }

void E5150::PIT::setOperationAccessForCounter (const unsigned int counterIndex, const uint8_t controlWord)
{
	if (isRL0Set(controlWord))
	{
		if (isRL1Set(controlWord))
		{
			m_counters[counterIndex].accessOperation = ACCESS_OPERATION::LSB_MSB;
			m_counters[counterIndex].readStatus = OPERATION_STATUS::LSB;
			m_counters[counterIndex].writeStatus = OPERATION_STATUS::LSB;
		}
		else
		{
			m_counters[counterIndex].accessOperation = ACCESS_OPERATION::LSB_ONLY;
			m_counters[counterIndex].readStatus = OPERATION_STATUS::LSB;
			m_counters[counterIndex].writeStatus = OPERATION_STATUS::LSB;
		}
	}
	else
	{
		if (isRL1Set(controlWord))
		{
			m_counters[counterIndex].accessOperation = ACCESS_OPERATION::MSB_ONLY;
			m_counters[counterIndex].readStatus = OPERATION_STATUS::MSB;
			m_counters[counterIndex].writeStatus = OPERATION_STATUS::MSB;
		}
		else
			m_counters[counterIndex].latchedValue = m_counters[counterIndex].counterValue.word;
	}
}

//I implement the modes in pit.cpp so that they are in the compilation unit of the pit and the compiler can optimmize call and have the
//possibility to inline.
/* *** IMPLEMENTING MODES *** */
E5150::PIT::MODE::MODE(Counter& relatedCounter): m_relatedCounter(relatedCounter)
{}

void E5150::PIT::MODE::clock (void) {}
void E5150::PIT::MODE::writeOperation (const uint8_t count) {}
void E5150::PIT::MODE::actionOnEnable (void) {}

//TODO: What happens when the mode change while the counter is counting ?
void E5150::PIT::MODE::enable (void)
{
	m_relatedCounter.mode = this;
	actionOnEnable();
}

/* *** IMPLEMENTING MODE0 *** */
void E5150::PIT::MODE0::actionOnEnable()
{
	m_relatedCounter.outputValue = OUTPUT_VALUE::LOW;
	m_relatedCounter.isCounting = false;
}

//TODO: is it relevant to emulate the behaviour of the gate input ?
void E5150::PIT::MODE0::clock()
{
	if (m_relatedCounter.gateValue == OUTPUT_VALUE::HIGH)
	{
		if (m_relatedCounter.outputValue == OUTPUT_VALUE::LOW)
		{
			if (m_relatedCounter.counterValue.word == 0)
				m_relatedCounter.outputValue = OUTPUT_VALUE::HIGH;
		}

		--m_relatedCounter.counterValue.word;
	}
	#ifdef CLOCK_DEBUG
		std::cout << "gate low: clock has no effect" << std::endl;
		PAUSE;
	#endif
}

void E5150::PIT::MODE0::writeOperation(const uint8_t count)
{
	switch (m_relatedCounter.writeStatus)
	{
		case OPERATION_STATUS::LSB:
		{
			m_relatedCounter.isCounting = false;
			m_relatedCounter.counterValue.lsb = count;

			if (m_relatedCounter.accessOperation == ACCESS_OPERATION::LSB_MSB)
				m_relatedCounter.writeStatus = OPERATION_STATUS::MSB;
			else
			{
				m_relatedCounter.isCounting = true;
				m_relatedCounter.outputValue = OUTPUT_VALUE::LOW;
			}
		} break;

		case OPERATION_STATUS::MSB:
		{
			m_relatedCounter.counterValue.msb = count;
			m_relatedCounter.isCounting = true;
			m_relatedCounter.outputValue = OUTPUT_VALUE::LOW;

			if (m_relatedCounter.accessOperation == ACCESS_OPERATION::LSB_MSB)
				m_relatedCounter.writeStatus = OPERATION_STATUS::LSB;
			
		} break;
	}
}

/* *** MODE1 *** */
void E5150::PIT::MODE1::actionOnEnable()
{}

void E5150::PIT::MODE1::writeOperation(const uint8_t count)
{}

void E5150::PIT::MODE1::clock()
{}

/* *** MODE2 *** */
void E5150::PIT::MODE2::actionOnEnable()
{}

void E5150::PIT::MODE2::writeOperation(const uint8_t count)
{}

void E5150::PIT::MODE2::clock()
{}

/* *** MODE3 *** */
void E5150::PIT::MODE3::actionOnEnable()
{}

void E5150::PIT::MODE3::writeOperation(const uint8_t count)
{}

void E5150::PIT::MODE3::clock()
{}

/* *** MODE4 *** */
void E5150::PIT::MODE4::actionOnEnable()
{}

void E5150::PIT::MODE4::writeOperation(const uint8_t count)
{}

void E5150::PIT::MODE4::clock()
{}

/* *** MODE5 *** */
void E5150::PIT::MODE5::actionOnEnable()
{}

void E5150::PIT::MODE5::writeOperation(const uint8_t count)
{}

void E5150::PIT::MODE5::clock()
{}