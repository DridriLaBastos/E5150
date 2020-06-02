#include "floppy.hpp"

//The IBM PC doc says that the floppy driver adapter have I/O port from 0x3F0 to 0x3F7 which means a 3 lines
//address bus is used. But there is only 3 registers for this adaptater: the DOR at 0x3F2 and the register
//from the UPD365 at 0x3F4, 0x3F5. The registers are maped like that:
//0b00x --> nothing
//0b010 --> DOR
//0b10x --> FDC
E5150::Floppy::Floppy(E5150::PIC& pic, PORTS& ports): Component("Floppy Controller",0b111), m_pic(pic), m_phase(PHASE::COMMAND)
{
	PortInfos dorStruct;
	dorStruct.portNum = 0x3F2;
	dorStruct.component = this;

	PortInfos mainStatusStruct;
	mainStatusStruct.portNum = 0x3F4;
	mainStatusStruct.component = this;

	PortInfos dataRegStruct;
	dataRegStruct.portNum = 0x3F5;
	dataRegStruct.component = this;

	ports.connect(dorStruct);
	ports.connect(mainStatusStruct);
	ports.connect(dataRegStruct);
}

static bool isA0Set (const unsigned int localAddress) { return localAddress & 0b1; }
static bool isA1Set (const unsigned int localAddress) { return localAddress & 0b10; }
static bool isA2Set (const unsigned int localAddress) { return localAddress & 0b100; }

void E5150::Floppy::writeDOR(const uint8_t data)
{ m_dorRegister = data; }

void E5150::Floppy::writeDataRegister(const uint8_t data)
{

}

void E5150::Floppy::write	(const unsigned int localAddress, const uint8_t data)
{
	if (localAddress == 2)
		writeDOR(data);
	else if (localAddress == 5)
		writeDataRegister(data);
}

uint8_t E5150::Floppy::readDataRegister()
{
	m_dataRegister = m_resultCommandData[m_resultCommandDataToRead++];

	if (m_resultCommandDataToRead >= 5)
	{
		m_phase == PHASE::COMMAND;
		m_resultCommandDataToRead = -1;
	}
	
	return m_dataRegister;
}

uint8_t E5150::Floppy::readStatusRegister()
{
	switch (m_phase)
	{
	case PHASE::COMMAND:
		//TODO: check write from the data register
		break;
	
	case PHASE::RESULT:
		++m_resultCommandDataToRead;
		break;
	
	default:
		break;
	}
	return m_statusRegister;
}

uint8_t E5150::Floppy::read	(const unsigned int localAddress)
{
	uint8_t ret;//I don't initialized ret. If a wrong address is given, then the returned value of the read
				//operation will be undefined
	if (localAddress == 2)
		ret = m_dorRegister;
	else
	{
		if ((localAddress == 4) || (localAddress == 5))
			ret = (localAddress == 4) ? readStatusRegister() : readDataRegister();
	}

	return ret;
}