#include "pit.hpp"

E5150::PIT::PIT(PORTS& ports, PIC& connectedPIC): m_connectedPIC(connectedPIC)
{
	for (Counter& c: m_counters)
	{
		c.isCounting = false;
		c.readComplete = true;
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

void E5150::PIT::clock()
{
	if (m_counters[0].isCounting)
		clockForCounter0();
	
	/*if (m_counters[1].isCounting)
		clockForCounter1();
	
	if (m_counters[2].isCounting)
		clockForCounter2();*/
}

void E5150::PIT::clockForCounter0()
{
	OUTPUT_VALUE oldOutPutValue = m_counters[0].outputValue;
	//TODO: I don't like that. How to change this to m_counters[0].mode->clock() ?
	m_counters[0].mode->clock(m_counters[0]);

	//Going from low to high
	if ((m_counters[0].outputValue == OUTPUT_VALUE::HIGH) && (oldOutPutValue == OUTPUT_VALUE::LOW))
		m_connectedPIC.assertInteruptLine(PIC::IR0);
}

void E5150::PIT::write (const unsigned int address, const uint8_t data)
{
	const unsigned int localAddress = address & 0b11;

	if (localAddress == 0b11)
		writeControlWord(data);
	else
		writeCounter(localAddress, data);
}

void E5150::PIT::writeCounter (const unsigned int counterIndex, const uint8_t data)
{ m_counters[counterIndex].mode->writeToCounter(m_counters[counterIndex], data); }

uint8_t E5150::PIT::applyPICReadAlgorithm (Counter& counter, const unsigned int value)
{
	switch (counter.readStatus)
	{
		case OPERATION_STATUS::LSB:
		{
			if (counter.accessOperation == ACCESS_OPERATION::LSB_MSB)
				counter.readStatus = OPERATION_STATUS::MSB;

			return value & 0xFF;
		}
		
		case OPERATION_STATUS::MSB:
		{
			counter.readStatus = OPERATION_STATUS::LSB;

			if (counter.accessOperation == ACCESS_OPERATION::LSB_MSB)
				counter.readStatus = OPERATION_STATUS::LSB;

			return (value & 0xFF00) >> 8;
		}
	}
}

uint8_t E5150::PIT::readCounterLatchedValue (Counter& counter)
{ return applyPICReadAlgorithm(counter, counter.latchedValue); }

//TODO: blocking the count down
uint8_t E5150::PIT::readCounterDirectValue (Counter& counter)
{ return applyPICReadAlgorithm(counter, counter.counterValue.word); }

uint8_t E5150::PIT::read (const unsigned int address)
{
	const unsigned int localAddress = address & 0b11;
	uint8_t ret = 0;

	if (localAddress != 0b11)
	{
		Counter& counter = m_counters[address];
		ret = (counter.latchedValueIsAvailable) ? readCounterLatchedValue(counter) : readCounterDirectValue(counter);
	}
	
	return ret;
}

static unsigned int extractRegisterNumber (const uint8_t controlWord) { return (controlWord & 0b11000000) >> 6; }

void E5150::PIT::writeControlWord (const uint8_t controlWord)
{
	const unsigned int counterIndex = extractRegisterNumber(controlWord);

	setModeForCounter(counterIndex, controlWord);
	setOperationAccessForCounter(counterIndex, controlWord);
	m_counters[counterIndex].codedBCD = controlWord & 0b1;
}

static bool isM0Set (const uint8_t controlWord) { return controlWord & 0b10; }
static bool isM1Set (const uint8_t controlWord) { return controlWord & 0b100; }
static bool isM2Set (const uint8_t controlWord) { return controlWord & 0b1000; }

void E5150::PIT::setModeForCounter (const unsigned int counterIndex, const uint8_t controlWord)
{
	if (isM1Set(controlWord))
	{
		if (isM0Set(controlWord)) m_mode3.setForCounter(m_counters[counterIndex]);
		else m_mode2.setForCounter(m_counters[counterIndex]);
	}
	else
	{
		if (isM2Set(controlWord))
		{
			if (isM0Set(controlWord)) m_mode5.setForCounter(m_counters[counterIndex]);
			else m_mode2.setForCounter(m_counters[counterIndex]);
		}
		else
		{
			if (isM0Set(controlWord)) m_mode1.setForCounter(m_counters[counterIndex]);
			else m_mode0.setForCounter(m_counters[counterIndex]);
		}
	}
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
//TODO: What happens when the mode change while the counter is counting ?
void E5150::PIT::MODE::setForCounter (E5150::PIT::Counter& counter)
{
	counter.mode = this;
	actionForSet(counter);
}

/* *** MODE 0 *** */
void E5150::PIT::MODE0::actionForSet(Counter& counter)
{
	counter.outputValue = OUTPUT_VALUE::LOW;
	counter.isCounting = false;
}

void E5150::PIT::MODE0::writeToCounter(Counter& counter, const uint8_t count)
{
	switch (counter.writeStatus)
	{
		case OPERATION_STATUS::LSB:
		{
			counter.isCounting = false;
			counter.counterValue.lsb = count;

			if (counter.accessOperation == ACCESS_OPERATION::LSB_MSB)
				counter.writeStatus = OPERATION_STATUS::MSB;
			else
				counter.isCounting = true;
		} break;

		case OPERATION_STATUS::MSB:
		{
			if (counter.accessOperation == ACCESS_OPERATION::MSB_ONLY)
				counter.isCounting = false;

			counter.counterValue.msb = count;

			if (counter.accessOperation == ACCESS_OPERATION::LSB_MSB)
				counter.writeStatus = OPERATION_STATUS::LSB;
			
			counter.isCounting = true;
		} break;
	}
}

void E5150::PIT::MODE0::clock(Counter& counter)
{
	if (counter.outputValue == OUTPUT_VALUE::LOW)
	{
		if (counter.counterValue.word == 0)
			counter.outputValue = OUTPUT_VALUE::HIGH;
	}

	--counter.counterValue.word;
}

/* *** MODE1 *** */
void E5150::PIT::MODE1::actionForSet(Counter& counter)
{}

void E5150::PIT::MODE1::writeToCounter(Counter& counter, const uint8_t count)
{}

void E5150::PIT::MODE1::clock(Counter& counter)
{}

/* *** MODE2 *** */
void E5150::PIT::MODE2::actionForSet(Counter& counter)
{}

void E5150::PIT::MODE2::writeToCounter(Counter& counter, const uint8_t count)
{}

void E5150::PIT::MODE2::clock(Counter& counter)
{}

/* *** MODE3 *** */
void E5150::PIT::MODE3::actionForSet(Counter& counter)
{}

void E5150::PIT::MODE3::writeToCounter(Counter& counter, const uint8_t count)
{}

void E5150::PIT::MODE3::clock(Counter& counter)
{}

/* *** MODE4 *** */
void E5150::PIT::MODE4::actionForSet(Counter& counter)
{}

void E5150::PIT::MODE4::writeToCounter(Counter& counter, const uint8_t count)
{}

void E5150::PIT::MODE4::clock(Counter& counter)
{}

/* *** MODE5 *** */
void E5150::PIT::MODE5::actionForSet(Counter& counter)
{}

void E5150::PIT::MODE5::writeToCounter(Counter& counter, const uint8_t count)
{}

void E5150::PIT::MODE5::clock(Counter& counter)
{}