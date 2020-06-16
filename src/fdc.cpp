#include "fdc.hpp"

//The IBM PC doc says that the floppy driver adapter have I/O port from 0x3F0 to 0x3F7 which means a 3 lines
//address bus is used. But there is only 3 registers for this adaptater: the DOR at 0x3F2 and the register
//from the UPD365 at 0x3F4, 0x3F5. The registers are maped like that:
//0b00x --> nothing
//0b010 --> DOR
//0b10x --> FDC
E5150::Floppy::Floppy(E5150::PIC& pic, PORTS& ports):
	Component("Floppy Controller",0b111), m_pic(pic), m_phase(PHASE::COMMAND), m_statusRegisterRead(false)
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

	m_commands[0]  = &invalid;          m_commands[1]  = &scanEqual;      m_commands[2]  = &readATrack;
	m_commands[3]  = &specify;          m_commands[4]  = &senseDriveStat; m_commands[5]  = &writeData;
	m_commands[6]  = &readData;         m_commands[7]  = &recalibrate;    m_commands[8]  = &senseInterruptStatus;
	m_commands[9]  = &writeDeletedData; m_commands[10] = &scanLEQ;        m_commands[11] = &readID;
	m_commands[12] = &readDeletedData;  m_commands[13] = &formatTrack;    m_commands[14] = &formatTrack;
	m_commands[15] = &seek;

	//TODO: search more info of the init state of the status register. For now is is set to the status:
	// + all drives in seek mode
	// + non DMA mode (but there is a DMA : more serch must be done)
	// + D6 = 0: data transfert is from the processor to the controller
	// + D7 = 1: data register ready to receive(/send)data from the processor(/to the processor)
	m_statusRegister = 0b10 << 6;
}

void E5150::Floppy::writeDOR(const uint8_t data)
{ m_dorRegister = data; }

void E5150::Floppy::switchPhase (void)
{
	switch (m_phase)
	{
		case PHASE::COMMAND:
		{
			m_phase = PHASE::EXECUTION;
			//clear the last bits without touching the other ones
			m_statusRegister &= ~(1 << 7); 
		} break;

		case PHASE::EXECUTION:
		{
			m_phase = PHASE::RESULT;
			m_statusRegister |= (0b11 << 6);
		} break;

		case PHASE::RESULT:
		{
			m_phase = PHASE::COMMAND;
			m_statusRegister &= ~(1 << 6);
			m_statusRegister |= (1 << 7);
		} break;
	}
}

static bool statusRegisterAllowsWritingInCommandPhase(const uint8_t statusRegister)
{ return (statusRegister & (0b10 << 6)) >> 6 & 0b10; }

void E5150::Floppy::writeDataRegisterCommandPhase (const uint8_t data)
{
	static bool firstCommandWorld = true;
	if (statusRegisterAllowsWritingInCommandPhase(m_statusRegister) && m_statusRegisterRead)
	{
		if (firstCommandWorld)
		{
			uint8_t commandIndex = data & 0b1111;

			//The first four digits of the first world identify the command except when the value is equals to 9 or 13
			//9 and 13 both identifies 2 commands, so by adding the fifth bit wa can select one of both commands
			if ((commandIndex == 9) || (commandIndex == 13))
				commandIndex += (data & 0b10000) >> 5;
			
			if (firstCommandWorld)
				m_selectedCommand = commandIndex;
		}

		if (m_commands[m_selectedCommand]->configure(data))
		{
			switchPhase();
			firstCommandWorld = true;
		}
		
		m_statusRegisterRead = false;
		firstCommandWorld = false;
	}
}

void E5150::Floppy::writeDataRegisterExecutionPhase (const uint8_t data)
{
	//TODO: what happens here ?
}

static bool statusRegisterAllowsWritingInResultPhase(const uint8_t statusRegister)
{ return (statusRegister & (0b11 << 6)) >> 6; }

void E5150::Floppy::writeDataRegisterResultPhase (const uint8_t data)
{
	if (statusRegisterAllowsWritingInResultPhase(m_statusRegister))
	{

	}
}

void E5150::Floppy::writeDataRegister(const uint8_t data)
{
	switch (m_phase)
	{
		case PHASE::COMMAND:
		writeDataRegisterCommandPhase(data);
		break;

		default:
			//TODO: what happens in this case
			break;
	}
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
	const auto [result,readDone] = m_commands[m_selectedCommand]->readResult();

	if (readDone)
		m_phase = PHASE::COMMAND;
	
	return result;
}

uint8_t E5150::Floppy::readStatusRegister()
{
	m_statusRegisterRead = true;
	switch (m_phase)
	{
	case PHASE::COMMAND:
		//TODO: check write from the data register
		break;
	
	case PHASE::RESULT:
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