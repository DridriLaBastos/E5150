#include "fdc.hpp"

//mmmh...
E5150::FDC* fdc = nullptr;

//The IBM PC doc says that the floppy driver adapter have I/O port from 0x3F0 to 0x3F7 which means a 3 lines
//address bus is used. But there is only 3 registers for this adaptater: the DOR at 0x3F2 and the register
//from the UPD365 at 0x3F4, 0x3F5. The registers are maped like that:
//0b00x --> nothing
//0b010 --> DOR
//0b10x --> FDC
E5150::FDC::FDC(E5150::PIC& pic, PORTS& ports):
	Component("Floppy Controller",0b111), m_pic(pic), m_phase(PHASE::COMMAND), m_statusRegisterRead(false)
{
	fdc = this;
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

unsigned int E5150::FDC::onClock ()
{
	return m_commands[m_selectedCommand]->exec();
}

void E5150::FDC::clock()
{
	static unsigned int passClocks = 0;
	static const unsigned int clockForOneMs = 8;//8 clocks for 1 ms at 8MHz

	if (passClocks-- == 0)
	{
		if (m_phase == PHASE::EXECUTION)
			passClocks = onClock() * clockForOneMs;
	}
}

void E5150::FDC::writeDOR(const uint8_t data)
{ m_dorRegister = data; }

void E5150::FDC::switchToCommandMode (void)
{
	m_phase = PHASE::COMMAND;
	m_statusRegister &= ~(1 << 6);
	m_statusRegister |= (1 << 7);
}

void E5150::FDC::switchToExecutionMode (void)
{
	m_phase = PHASE::EXECUTION;
	//clears the last bits without touching the other ones
	m_statusRegister &= ~(1 << 7);
	m_statusRegister |= (1 << 6);
}

void E5150::FDC::switchToResultMode (void)
{
	m_phase = PHASE::RESULT;
	m_statusRegister |= (1 << 6);
	m_statusRegister &= ~(1 << 7);
}

//TODO: check again the value of the bits
void E5150::FDC::switchPhase (void)
{
	switch (m_phase)
	{
		case PHASE::COMMAND:
			switchToExecutionMode();
			break;

		case PHASE::EXECUTION:
			switchToResultMode();
			break;

		case PHASE::RESULT:
			switchToCommandMode();
			break;
	}
}

void E5150::FDC::writeDataRegister(const uint8_t data)
{
	static bool firstCommandWorld = true;
	if (m_statusRegisterRead)
	{
		if (firstCommandWorld)
		{
			uint8_t commandIndex = data & 0b1111;

			//The first four digits of the first world identify the command except when the value equals 9 or 13
			//9 and 13 both identifies 2 commands, so by adding the fifth bit we can select one of both commands
			if ((commandIndex == 9) || (commandIndex == 13))
				commandIndex += (data & 0b10000) >> 5;
			
			if (firstCommandWorld)
				m_selectedCommand = commandIndex;
		}

		m_statusRegisterRead = false;
		firstCommandWorld = m_commands[m_selectedCommand]->configure(data);
	}
}

static bool fdcAllowsWritingToDataRegister (const uint8_t statusRegister)
{ return (statusRegister & (1 << 7)) >> 7; }

void E5150::FDC::write	(const unsigned int localAddress, const uint8_t data)
{
	if (localAddress == 2)
		writeDOR(data);
	else if (localAddress == 5)
	{
		if (fdcAllowsWritingToDataRegister(m_statusRegister))
			writeDataRegister(data);
	}
}

uint8_t E5150::FDC::readDataRegister()
{
	if (m_statusRegisterRead)
	{
		const auto [result,readDone] = m_commands[m_selectedCommand]->readResult();

		if (readDone)
			switchPhase();
		
		m_dataRegister = result;
		m_statusRegisterRead = false;
	}
	
	return m_dataRegister;
}

uint8_t E5150::FDC::readStatusRegister()
{
	m_statusRegisterRead = true;
	return m_statusRegister;
}

static bool fdcAllowsReadingDataRegister(const uint8_t statusRegister)
{ return (statusRegister & (0b11 << 6)) == (0b11 << 6); }

uint8_t E5150::FDC::read	(const unsigned int localAddress)
{
	uint8_t ret;//I don't initialized ret. If a wrong addres is given, then the returned value of the read
				//operation will be undefined
	if (localAddress == 2)
		ret = m_dorRegister;
	else
	{
		if ((localAddress == 4) || (localAddress == 5))
		{
			if (localAddress == 4)
				ret = readStatusRegister();
			else
			{
				if (fdcAllowsReadingDataRegister(m_statusRegister))
					ret = readDataRegister();
			}
		}
	}

	return ret;
}

///////////////////////////////////
/*** IMPLEMENTING THE COMMANDS ***/
///////////////////////////////////
E5150::FDC::Command::Command(const unsigned int configurationWorldNumber, const unsigned int resultWorldNumber):
	m_configurationWords(configurationWorldNumber), m_resultWords(resultWorldNumber)
{}

bool E5150::FDC::Command::configure (const uint8_t data)
{
	static unsigned int configurationStep = 0;
	m_configurationWords[configurationStep++] = data;
	configurationStep %= m_configurationWords.size();

	if (configurationStep == 0)
	{
		fdc->switchToExecutionMode();
		onConfigureFinish();
	}

	return (configurationStep == 0);
}

std::pair<uint8_t,bool> E5150::FDC::Command::readResult (void)
{
	static unsigned int readingStep = 0;
	const uint8_t ret = m_resultWords[readingStep++];
	return {ret, (readingStep % m_resultWords.size()) == 0};
}

E5150::FDC::COMMAND::SenseDriveStatus::SenseDriveStatus(): Command(2,1)
{}

E5150::FDC::COMMAND::Seek::Seek(): Command(3,0)
{}

E5150::FDC::COMMAND::Invalid::Invalid(): Command(1,1)
{ m_resultWords[0] = 0x80; }

E5150::FDC::COMMAND::Specify::Specify(): Command(3,0)
{}

void E5150::FDC::COMMAND::Specify::onConfigureFinish()
{
	const uint8_t SRTValue = (m_configurationWords[1] & (0b111 << 5)) >> 5;
	const uint8_t HUTValue = m_configurationWords[1] & 0b111;
	const uint8_t HLTValue = (m_configurationWords[2] & ~1) >> 1;

	fdc->m_timers[TIMER::STEP_RATE_TIME] = SRTValue;
	fdc->m_timers[TIMER::HEAD_UNLOAD_TIME] = HUTValue;
	fdc->m_timers[TIMER::HEAD_LOAD_TIME] = HLTValue;
	
	fdc->switchToCommandMode();
}