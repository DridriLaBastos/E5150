#include "pit.hpp"

E5150::PIT::PIT(PORTS& ports, PIC& connectedPIC): m_connectedPIC(connectedPIC)
{
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
	execModeForCounter(m_counters[0]);

	//Going from low to high
	if ((m_counters[0].outputValue == OUTPUT_VALUE::HIGH) && (oldOutPutValue == OUTPUT_VALUE::LOW))
		m_connectedPIC.assertInteruptLine(PIC::IR0);
	
	--m_counters[0].counterValue.word;
}

void E5150::PIT::execModeForCounter (PIT::Counter& counter)
{
	switch (m_counters[0].mode)
	{
		case MODE::MODE0:
			clockMode0(counter);
			break;
		
		/*case MODE::MODE1:
			clockMode1(c);
			break;
		
		case MODE::MODE2:
			clockMode2(c);
			break;
		
		case MODE::MODE3:
			clockMode3(c);
			break;
		
		case MODE::MODE4:
			clockMode4(c);
			break;
		
		case MODE::MODE5:
			clockMode5(c);
			break;*/
	}
}

void E5150::PIT::clockMode0 (Counter& counter)
{ if (counter.counterValue.word == 0) counter.outputValue = OUTPUT_VALUE::HIGH; }

void E5150::PIT::write (const unsigned int address, const uint8_t data)
{
	const unsigned int localAddress = address & 0b11;

	if (localAddress == 0b11)
		writeControlWord(data);
	else
		writeCounter(localAddress, data);
}

void E5150::PIT::writeCounter (const unsigned int counterIndex, const uint8_t data)
{
	switch (m_counters[counterIndex].writeStatus)
	{
		case OPERATION_STATUS::LSB:
		{
			m_counters[counterIndex].counterValue.lsb = data;

			if (m_counters[counterIndex].accessOperation == ACCESS_OPERATION::LSB_MSB)
				m_counters[counterIndex].writeStatus = OPERATION_STATUS::MSB;
		} break;

		case OPERATION_STATUS::MSB:
		{
			m_counters[counterIndex].counterValue.msb = data;

			if (m_counters[counterIndex].accessOperation == ACCESS_OPERATION::LSB_MSB)
				m_counters[counterIndex].writeStatus = OPERATION_STATUS::LSB;
		} break;
	}
}

uint8_t E5150::PIT::read (const unsigned int address)
{
	const unsigned int localAddress = address & 0b11;

	if (localAddress != 0b11)
	{
		switch (m_counters[localAddress].readStatus)
		{
		case OPERATION_STATUS::LSB:
			m_counters[localAddress].readStatus = OPERATION_STATUS::MSB;
			return m_counters[localAddress].latchedValue & 0xFF;
		
		case OPERATION_STATUS::MSB:
			m_counters[localAddress].readStatus = OPERATION_STATUS::LSB;
			return (m_counters[localAddress].latchedValue & 0xFF00) >> 8;
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
			m_counters[counterIndex].mode = isM0Set(controlWord) ? MODE::MODE5 : MODE::MODE4;
		else
			m_counters[counterIndex].mode = isM0Set(controlWord) ? MODE::MODE1 : MODE::MODE0;
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