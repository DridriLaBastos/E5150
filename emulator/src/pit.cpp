#include "pit.hpp"

E5150::PIT::PIT(PIC& connectedPIC): m_connectedPIC(connectedPIC)
{

}

void E5150::PIT::write (const unsigned int address, const uint8_t data)
{
	const unsigned int localAddress = address & 0b11;

	if (localAddress == 0b11)
		writeControlWord(data);
}

uint8_t E5150::PIT::read (const unsigned int address)
{
	return 0;
}

static unsigned int extractRegisterNumber (const uint8_t controlWord) { return (controlWord & 0b11000000) >> 6; }

void E5150::PIT::writeControlWord (const uint8_t controlWord)
{
	const unsigned int counterIndex = extractRegisterNumber(controlWord);

	setModeForCounter(counterIndex, controlWord);
	setOperationAccessForCounter(counterIndex, controlWord);
	m_counters[counterIndex].codedBCD = controlWord & 0b1;
}

//TODO: revérifier tous ça parce que j'ai pu faire de la merde !!!
static bool isM0Set (const uint8_t controlWord) { return controlWord & 0b10; }
static bool isM1Set (const uint8_t controlWord) { return controlWord & 0b100; }
static bool isM2Set (const uint8_t controlWord) { return controlWord & 0b1000; }

void E5150::PIT::setModeForCounter (const unsigned int counterIndex, const uint8_t controlWord)
{
	if (isM1Set(controlWord))
		m_counters[counterIndex].mode = isM0Set(controlWord) ? MODE::MODE3 : MODE::MODE2;
	else
	{
		if (isM2Set(controlWord))
			m_counters[counterIndex].mode = isM0Set(controlWord) ? MODE::MODE4 : MODE::MODE5;
		else
			m_counters[counterIndex].mode = isM0Set(controlWord) ? MODE::MODE1 : MODE::MODE0;
	}
}

static bool isRL0Set (const uint8_t controlWord) { return controlWord & 0b10000; }
static bool isRL1Set (const uint8_t controlWord) { return controlWord & 0b100000; }

void E5150::PIT::setOperationAccessForCounter (const unsigned int counterIndex, const uint8_t controlWord)
{
	if (isRL0Set(controlWord))
		m_counters[counterIndex].accessOperation = isRL1Set(controlWord) ? ACCESS_OPERATION::LSB_MSB : ACCESS_OPERATION::LSB_MSB;
	else
		m_counters[counterIndex].accessOperation = isRL1Set(controlWord) ? ACCESS_OPERATION::MSB_ONLY : ACCESS_OPERATION::LATCHING;
}