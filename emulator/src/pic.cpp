#include "pic.hpp"

template <unsigned int _N>
unsigned int log2_2 (void)
{ return (_N == 1) ? 0 : (1 + log2_2<_N - 1>()); }

E5150::PIC::PIC(PORTS& ports, CPU& connectedCPU): m_connectedCPU(connectedCPU), m_initStatus(INIT_STATUS::UNINITIALIZED)
{
	PortInfos commandInfo;
	commandInfo.portNum = 0x20;
	commandInfo.component = this;

	PortInfos dataInfo;
	dataInfo.portNum = 0x21;
	dataInfo.component = this;

	ports.connect(commandInfo);   ports.connect(dataInfo);
}

uint8_t E5150::PIC::readA0_0 () const { return m_regs[m_nextRegisterToRead]; }
uint8_t E5150::PIC::readA0_1 () const { return m_regs[IMR]; }

uint8_t E5150::PIC::read(const unsigned int address)
{
	const unsigned int localAddress = address & 0b1;
	return localAddress ? readA0_1() : readA0_0();
}

static bool isICW1 (const uint8_t icw) { return icw & 0b10000; }
static bool isOCW3 (const uint8_t ocw) { return ocw & 0b1000; }

void E5150::PIC::writeA0_0 (const uint8_t data)
{
	std::cout << "Write to the PIC with A0 = 0" << std::endl;
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
	std::cout << "Write to the PIC with A0 = 1" << std::endl;
	if (m_initStatus != INIT_STATUS::INITIALIZED)
		handleInitSequence(data);
	else
		handleOCW1(data);
}

void E5150::PIC::write(const unsigned int address, const uint8_t data)
{
	const unsigned int localAddress = address & 0b1;

	if (localAddress == 0)
		writeA0_0(data);
	else
		writeA0_1(data);
}

void E5150::PIC::assertInteruptLine(const E5150::PIC::INTERRUPT_LINE irLine)
{
	/*if (m_initStatus == INIT_STATUS::INITIALIZED)
	{
		const unsigned int irLineIndex = log2_2<(unsigned int)irLine>();
		m_regs[static_cast<unsigned int>(REGISTER::IRR)] |= irLine;
	}*/
}

static bool isICW4Needed (const uint8_t icw1) { return icw1 & 0b1; }
static bool isInSingleMode (const uint8_t icw1) { return icw1 & 0b10; }
static bool isAddressCallIntervalOf4 (const uint8_t icw1) { return icw1 & 0b100; }
static bool isInLevelTriggeredMode (const uint8_t icw1) { return icw1 & 0b1000; }

void E5150::PIC::handleICW1 (const uint8_t icw1)
{
	//TODO: remove this
	if (!isICW4Needed(icw1))
		throw std::logic_error("ERROR: ICW1: ICW4 is needed to put the PIC in 8086/8088 mode because the IBM PC use an intel 8088");
	m_picInfo.icw4Needed = true;
	m_picInfo.singleMode = isInSingleMode(icw1);
	m_picInfo.addressInterval4 = isAddressCallIntervalOf4(icw1);
	m_picInfo.levelTriggered = isInLevelTriggeredMode(icw1);

	m_nextRegisterToRead = IRR;
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

void E5150::PIC::handleOCW2 (const uint8_t ocw2)
{
	//TODO: implement
}

static bool isRISSet (const uint8_t ocw3) { return ocw3 & 0b1; }
static bool isRRSet (const uint8_t ocw3) { return ocw3 & 0b10; }

void E5150::PIC::handleOCW3 (const uint8_t ocw3)
{
	if (isRRSet(ocw3))
		m_nextRegisterToRead = isRISSet(ocw3) ? ISR : IRR;
	//TODO: implement
}