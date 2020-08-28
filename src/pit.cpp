#include "pit.hpp"

/* Declaration of all the modes so that their addresses can be taken when setting a mode */
E5150::PIT::MODE0 E5150::PIT::mode0;
E5150::PIT::MODE1 E5150::PIT::mode1;
E5150::PIT::MODE2 E5150::PIT::mode2;
E5150::PIT::MODE3 E5150::PIT::mode3;
E5150::PIT::MODE4 E5150::PIT::mode4;
E5150::PIT::MODE5 E5150::PIT::mode5;

static unsigned int counterIndex = 0;

//From intel documentation : "Prior to initialization, the MODE, count, and output of all counters is undefined".
//Initialization here initialization by software. But, we are not in real life, and we want value for some variables.
//Specialy, we don't want to leave the pointer to the mode uninitialized, so it is set to 0.
E5150::PIT::Counter::Counter()
{
	isCounting = false;
	readComplete = true;
	index = counterIndex++;
	gateValue = OUTPUT_VALUE::HIGH;
	mode0.enable(*this);
}

E5150::PIT::PIT(PORTS& ports, PIC& connectedPIC): Component("PIT",ports,0x40,0b11), m_connectedPIC(connectedPIC)
{
	//TODO: gate value for counter 2
}

void E5150::PIT::toggleGateFor(const COUNTER counterNumber)
{
	Counter& counter = m_counters[static_cast<unsigned int>(counterNumber)];
	counter.gateValue = (counter.gateValue == OUTPUT_VALUE::LOW) ? OUTPUT_VALUE::HIGH : OUTPUT_VALUE::LOW;
}

//TODO: implement the one clock wait to confirm writing to the counter
void E5150::PIT::clock()
{
	for (size_t i = 0; i < m_counters.size(); ++i)
	{
		OUTPUT_VALUE oldOutputValue = m_counters[i].outputValue;
		m_counters[i].mode->clock(m_counters[i]);

		if (i == 0)
		{
			if ((m_counters[i].outputValue == OUTPUT_VALUE::HIGH) && (oldOutputValue == OUTPUT_VALUE::LOW))
				m_connectedPIC.assertInterruptLine(PIC::IR0);
		}
	}
}

void E5150::PIT::writeCounter (const unsigned int counterIndex, const uint8_t data)
{
	Counter& counter = m_counters[counterIndex];
	counter.mode->writeOperation(counter, data);
}

static bool isM0Set (const uint8_t controlWord) { return controlWord & 0b10; }
static bool isM1Set (const uint8_t controlWord) { return controlWord & 0b100; }
static bool isM2Set (const uint8_t controlWord) { return controlWord & 0b1000; }

void E5150::PIT::setModeForCounter (const unsigned int counterIndex, const uint8_t controlWord)
{
	Counter& counter = m_counters[counterIndex];
	if (isM1Set(controlWord))
	{
		if (isM0Set(controlWord)) mode3.enable(counter);
		else mode2.enable(counter);
	}
	else
	{
		if (isM2Set(controlWord))
		{
			if (isM0Set(controlWord)) mode5.enable(counter);
			else mode4.enable(counter);
		}
		else
		{
			if (isM0Set(controlWord)) mode1.enable(counter);
			else mode0.enable(counter);
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
{ return applyPICReadAlgorithm(counter, counter.value.word); }

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
			m_counters[counterIndex].latchedValue = m_counters[counterIndex].value.word;
	}
}

//I implement the modes in pit.cpp so that they are in the compilation unit of the pit and the compiler can optimmize call and have the
//possibility to inline.
/* *** IMPLEMENTING MODES *** */

/* *** IMPLEMENTING MODE0 *** */
//When the the mode is set to counter0, the counter stops counting and wait for a rewriting of the count
//to start counting down
void E5150::PIT::MODE0::enable (Counter& counter)
{
	counter.mode = this;
	counter.outputValue = OUTPUT_VALUE::LOW;
	counter.isCounting = false;
	DEBUG("PIT: COUNTER{}: MODE0: set", counter.index);
}

//TODO: is it relevant to emulate the behaviour of the gate input ?
void E5150::PIT::MODE0::clock(Counter& counter)
{
	if (counter.gateValue == OUTPUT_VALUE::HIGH)
	{
		if (counter.outputValue == OUTPUT_VALUE::LOW)
		{
			if (counter.value.word == 0)
				counter.outputValue = OUTPUT_VALUE::HIGH;
		}

		--counter.value.word;
	}
	#ifdef CLOCK_DEBUG
	else
		DEBUG("PIT: COUNTER{0}: MODE0: gate low, clock has no effects",counter.index);
	#endif
}

//Intel documentation sais that write the first byte stops the counter and writing the second byte starts
//the counter in the definition of mode0. But is this behaviour only for mode0 or is this for all the modes ?
void E5150::PIT::MODE0::writeOperation(Counter& counter, const uint8_t count)
{
	switch (counter.writeStatus)
	{
		case OPERATION_STATUS::LSB:
		{
			counter.isCounting = false;
			counter.value.lsb = count;

			if (counter.accessOperation == ACCESS_OPERATION::LSB_MSB)
				counter.writeStatus = OPERATION_STATUS::MSB;
			else
			{
				counter.isCounting = true;
				counter.outputValue = OUTPUT_VALUE::LOW;
			}
		} break;

		case OPERATION_STATUS::MSB:
		{
			counter.value.msb = count;
			counter.isCounting = true;
			counter.outputValue = OUTPUT_VALUE::LOW;

			if (counter.accessOperation == ACCESS_OPERATION::LSB_MSB)
				counter.writeStatus = OPERATION_STATUS::LSB;
			
		} break;
	}
}

/* *** MODE1 *** */
void E5150::PIT::MODE1::enable(Counter& counter)
{}

void E5150::PIT::MODE1::clock(Counter& counter)
{}

void E5150::PIT::MODE1::writeOperation(Counter& counter, const uint8_t count)
{}

/* *** MODE2 *** */
void E5150::PIT::MODE2::enable(Counter& counter)
{}

void E5150::PIT::MODE2::clock(Counter& counter)
{}

void E5150::PIT::MODE2::writeOperation(Counter& counter, const uint8_t count)
{}

/* *** MODE3 *** */
void E5150::PIT::MODE3::enable(Counter& counter)
{}

void E5150::PIT::MODE3::clock(Counter& counter)
{}

void E5150::PIT::MODE3::writeOperation(Counter& counter, const uint8_t count)
{}

/* *** MODE4 *** */
void E5150::PIT::MODE4::enable(Counter& counter)
{}

void E5150::PIT::MODE4::clock(Counter& counter)
{}

void E5150::PIT::MODE4::writeOperation(Counter& counter, const uint8_t count)
{}

/* *** MODE5 *** */
void E5150::PIT::MODE5::enable(Counter& counter)
{}

void E5150::PIT::MODE5::clock(Counter& counter)
{}

void E5150::PIT::MODE5::writeOperation(Counter& counter, const uint8_t count)
{}