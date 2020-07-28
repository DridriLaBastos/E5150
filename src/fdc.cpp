#include "fdc.hpp"

//mmmh...
static E5150::FDC* fdc = nullptr;

//The IBM PC doc says that the floppy driver adapter have I/O port from 0x3F0 to 0x3F7 which means a 3 lines
//address bus is used. But there is only 3 registers for this adaptater: the DOR at 0x3F2 and the register
//from the UPD365 at 0x3F4, 0x3F5. The registers are maped like that:
//0b00x --> nothing
//0b010 --> DOR
//0b10x --> FDC
E5150::FDC::FDC(E5150::PIC& pic, PORTS& ports):
	Component("Floppy Controller",0b111), m_pic(pic), m_dorRegister(0), m_dataRegister(0), m_phase(PHASE::COMMAND), m_passClock(0), m_statusRegisterRead(false)
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
	m_commands[9]  = &writeDeletedData; m_commands[10] = &readID;         m_commands[11] = &scanLEQ;
	m_commands[12] = &readDeletedData;  m_commands[13] = &formatTrack;    m_commands[14] = &scanHEQ;
	m_commands[15] = &seek;

	//TODO: search more info of the init state of the status register. For now is is set to the status:
	// + all drives in seek mode
	switchToCommandMode();
}

void E5150::FDC::waitClock (const unsigned int clock) { m_passClock += clock; std::cout << "FDC will wait " << m_passClock << " clock(s)\n"; }
void E5150::FDC::waitMicro (const unsigned int microseconds) { waitClock(microseconds*8); }
void E5150::FDC::waitMilli (const unsigned int milliseconds) { waitMicro(milliseconds*1000); }

void E5150::FDC::makeBusy () { m_statusRegister |= (1 << 4); }
void E5150::FDC::makeAvailable () { m_statusRegister &= ~(1 << 4); }

void E5150::FDC::setSeekStatusOn (const FLOPPY_DRIVE drive) { m_statusRegister |= static_cast<unsigned>(drive); }
void E5150::FDC::resetSeekStatusOn (const FLOPPY_DRIVE drive) { m_statusRegister &= ~(static_cast<unsigned>(drive)); }

void E5150::FDC::makeDataRegisterReady (void) { m_statusRegister |= (1 << 7); }
void E5150::FDC::makeDataRegisterNotReady (void) { m_statusRegister &= ~(1 << 7); }
void E5150::FDC::makeDataRegisterInReadMode (void) { m_statusRegister |= 1 << 6; }
void E5150::FDC::makeDataRegisterInWriteMode (void) { m_statusRegister &= ~(1 << 6); }

bool E5150::FDC::dataRegisterReady (void) const { return m_statusRegister & (1 << 7); }
bool E5150::FDC::dataRegisterInReadMode (void) const { return m_statusRegister & (1 << 6); }
bool E5150::FDC::dataRegisterInWriteMode (void) const { return !dataRegisterInReadMode(); }
bool E5150::FDC::statusRegisterAllowReading (void) const { return dataRegisterReady() && dataRegisterInReadMode() && m_statusRegisterRead; }
bool E5150::FDC::statusRegisterAllowWriting (void) const { return dataRegisterReady() && dataRegisterInWriteMode() && m_statusRegisterRead; }

//ok
void E5150::FDC::clock()
{
	if (m_passClock == 0)
	{
		if (m_phase == PHASE::EXECUTION)
			m_commands[m_selectedCommand]->exec();
	}
	else
	{
		--m_passClock;
		std::cout << m_passClock << " to wait\n";
	}
}

//TODO: if selected while motor not on, does it unselect the previously selected floppy ?
void E5150::FDC::writeDOR(const uint8_t data)
{
	static Floppy100* previouslySelected = nullptr;

	m_floppyDrives[0].setMotorSpinning(data & (1 << 4));
	m_floppyDrives[1].setMotorSpinning(data & (1 << 5));
	m_floppyDrives[2].setMotorSpinning(data & (1 << 6));
	m_floppyDrives[3].setMotorSpinning(data & (1 << 7));

	if (m_floppyDrives[data & 0b11].select())
	{
		if (previouslySelected != nullptr)
			previouslySelected->unselect();
		previouslySelected = &m_floppyDrives[data & 0b11];
	}
}

void E5150::FDC::switchToCommandMode (void)
{
	m_phase = PHASE::COMMAND;
	makeDataRegisterReady();
	makeDataRegisterInWriteMode();
	std::cout << "FDC switched to command mode\n";
}

void E5150::FDC::switchToExecutionMode (void)
{
	m_phase = PHASE::EXECUTION;
	//clears the last bits without touching the other ones
	makeDataRegisterNotReady();
	std::cout << "FDC switched to execution mode\n";
}

void E5150::FDC::switchToResultMode (void)
{
	m_phase = PHASE::RESULT;
	makeDataRegisterReady();
	makeDataRegisterInReadMode();
	std::cout << "FDC switched to result mode\n";
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
	if (firstCommandWorld)
	{
		uint8_t commandIndex = data & 0b1111;

		//The first four digits of the first world identify the command except when the value equals 9 or 13
		//9 and 13 both identifies 2 commands, so by adding the fifth bit we can select one of both commands
		if ((commandIndex == 9) || (commandIndex == 13))
		{
			const bool hasCommandIndexOffset = data & 0b10000;

			if (hasCommandIndexOffset)
				commandIndex += (commandIndex == 9) ? 2 : 1;
		}
		
		if (firstCommandWorld)
			m_selectedCommand = commandIndex;
		
		DEBUG("Selecting command '{}'",m_commands[m_selectedCommand]->m_name);
	}
	
	firstCommandWorld = m_commands[m_selectedCommand]->configure(data);
	m_statusRegisterRead = false;
}

void E5150::FDC::write	(const unsigned int localAddress, const uint8_t data)
{
	switch (localAddress)
	{
		case 2:
			writeDOR(data);
			return;
		
		case 5:
		{
			if (m_statusRegisterRead)
			{
				if (dataRegisterInWriteMode())
				{
					writeDataRegister(data);
					return;
				}
				else
					DEBUG("Data register {}", dataRegisterReady() ? "not in write mode" : "not ready");
			}
			else
				DEBUG("Status register not read before writing to data register");
		}

		default:
			DEBUG("Cannot write address {:b}. Address should be 0x3F2 for DOR or 0x3F5 for data register",localAddress);
	}

	DEBUG("Writing not done");
}

uint8_t E5150::FDC::readDataRegister()
{
	const auto [result,readDone] = m_commands[m_selectedCommand]->readResult();

	if (readDone)
		switchPhase();
	
	m_dataRegister = result;
	m_statusRegisterRead = false;
	return m_dataRegister;
}

uint8_t E5150::FDC::readStatusRegister()
{
	m_statusRegisterRead = true;
	return m_statusRegister;
}

uint8_t E5150::FDC::read	(const unsigned int localAddress)
{
	switch (localAddress)
	{
		case 2:
			return m_dorRegister;
		
		case 4:
			return readStatusRegister();
		
		case 5:
		{
			if (m_statusRegisterRead)
			{
				if (dataRegisterInReadMode())
					return readDataRegister();
				else
					DEBUG("Data regisrer {}.",dataRegisterReady() ? "in write mode" : "not ready");
			}
			else
				DEBUG("Status register not read before reading data register");
		}
		
		default:
			DEBUG("Cannot read address {:b}. Address should be 0x3F2 for DOR, 0x3F4 or 0x3F5 for status/data register",localAddress);
	}

	DEBUG("Value outputed will be undetermined");
}

///////////////////////////////////
/*** IMPLEMENTING THE COMMANDS ***/
///////////////////////////////////
E5150::FDC::Command::Command(const std::string& name, const unsigned int configurationWorldNumber, const unsigned int resultWorldNumber):
	m_name(name), m_configurationWords(configurationWorldNumber), m_resultWords(resultWorldNumber),m_configurationStep(0)
{}

bool E5150::FDC::Command::configure (const uint8_t data)
{
	if (m_configurationStep == 0)
	{
		onConfigureBegin();

		if (m_checkMFM)
		{
			if (!(data & (1 << 6)))
				throw std::logic_error("FM mode is not supported with the floppy drive");
		}
	}

	m_configurationWords[m_configurationStep++] = data;

	const bool configurationFinished = m_configurationStep == m_configurationWords.size();

	if (configurationFinished)
	{
		fdc->switchToExecutionMode();
		m_configurationStep = 0;

		if (m_saveHDS_DSx)
		{
			//TODO: do I really want it that way ?
			//TODO: save HDS
			//TODO: what happens if the floppy is not ready
			m_floppyDrive = m_configurationWords[1] & 0b11;
		}

		onConfigureFinish();
	}

	return configurationFinished;
}

std::pair<uint8_t,bool> E5150::FDC::Command::readResult (void)
{
	static unsigned int readingStep = 0;
	const uint8_t ret = m_resultWords[readingStep++];
	return {ret, (readingStep % m_resultWords.size()) == 0};
}

void E5150::FDC::Command::onConfigureFinish()
{  }

void E5150::FDC::Command::onConfigureBegin() {}
void E5150::FDC::Command::exec() {}

///////////////////////////////////
/*** IMPLEMENTING READ DATA ***/
///////////////////////////////////

void E5150::FDC::COMMAND::ReadData::loadHeads()
{
	/*if (!fdc->m_floppyDrives[m_floppyDrive].areHeadsLoaded())
	{
		fdc->m_floppyDrives[m_floppyDrive].pepare();
		fdc->waitMilli(fdc->m_timers[TIMER::HEAD_LOAD_TIME]*16);
	}*/
	
	m_status = STATUS::READ_DATA;

}

void E5150::FDC::COMMAND::ReadData::exec()
{
	switch (m_status)
	{
		case STATUS::LOADING_HEADS:
			loadHeads();
			break;
		
		case STATUS::READ_DATA:
			break;
	}
}

//////////////////////////////
/*** IMPLEMENTING READ ID ***/
//////////////////////////////

E5150::FDC::COMMAND::ReadID::ReadID(): Command("Read ID",2) {}

//TODO: timing !
void E5150::FDC::COMMAND::ReadID::exec()
{
	const unsigned int selectedHead = (m_configurationWords[1] & 0b100) >> 2;
	const auto& [track, sector] = fdc->m_floppyDrives[selectedHead].getID();

	m_resultWords[0] = fdc->m_STRegisters[0];
	m_resultWords[1] = fdc->m_STRegisters[1];
	m_resultWords[2] = fdc->m_STRegisters[2];
	m_resultWords[3] = track;
	m_resultWords[4] = selectedHead;
	m_resultWords[5] = sector;
	m_resultWords[6] = 0; //TODO: why is this set to 1 un bochs ?

	//TODO: investigate timing : I assume one write per clock, it might be more or less
	//In bochs this took 11 111 clocks
	fdc->waitClock(7);
	fdc->switchToResultMode();
}


E5150::FDC::COMMAND::SenseDriveStatus::SenseDriveStatus(): Command("Sense Drive Status",2,1)
{
	m_checkMFM = false;
	m_saveHDS_DSx = false;
}

//////////////////////////////
/*** IMPLEMENTING SPECIFY ***/
//////////////////////////////

E5150::FDC::COMMAND::Specify::Specify(): Command("Specify",3,0)
{ m_checkMFM = false;   m_saveHDS_DSx = false; }

void E5150::FDC::COMMAND::Specify::onConfigureFinish()
{
	const uint8_t SRTValue = m_configurationWords[1] >> 4;
	const uint8_t HUTValue = m_configurationWords[1] & 0xF;
	const uint8_t HLTValue = m_configurationWords[2] >> 1;

	if (HUTValue == 0)
		throw std::logic_error("HUT cannot be 0");
	
	if (SRTValue == 0)
		throw std::logic_error("STR value cannot be 0");
	
	if ((HLTValue == 0) || (HLTValue == 0xFF))
		throw std::logic_error("HLT value should be in [0x1, 0xFE]");

	fdc->m_timers[TIMER::STEP_RATE_TIME] = SRTValue;
	fdc->m_timers[TIMER::HEAD_UNLOAD_TIME] = HUTValue;
	fdc->m_timers[TIMER::HEAD_LOAD_TIME] = HLTValue;

	DEBUG("SRT Value set to {}",fdc->m_timers[TIMER::STEP_RATE_TIME]);
	DEBUG("HUT Value set to {}",fdc->m_timers[TIMER::HEAD_UNLOAD_TIME]);
	DEBUG("HLT Value set to {}",fdc->m_timers[TIMER::HEAD_LOAD_TIME]);
	
	fdc->switchToCommandMode();
	//TODO: investigate timing : I assume one write per clock, it might be more or less
	fdc->waitClock(3);
}

//////////////////////////////
/*** IMPLEMENTING    SEEK ***/
//////////////////////////////

E5150::FDC::COMMAND::Seek::Seek(): Command("Seek",3,0), m_floppyToApply(0) { m_checkMFM = false; }

void E5150::FDC::COMMAND::Seek::onConfigureBegin ()
{ fdc->makeBusy(); }

void E5150::FDC::COMMAND::Seek::onConfigureFinish()
{
	if (fdc->m_floppyDrives[m_floppyToApply].isReady())
	{
		FLOPPY_DRIVE floppyDriveSeeking;

		switch (m_floppyDrive)
		{
			case 0: floppyDriveSeeking = FLOPPY_DRIVE::A; break;
			case 1: floppyDriveSeeking = FLOPPY_DRIVE::B; break;
			case 2: floppyDriveSeeking = FLOPPY_DRIVE::C; break;
			case 3: floppyDriveSeeking = FLOPPY_DRIVE::D; break;
		}

		m_floppyToApply |= static_cast<unsigned>(floppyDriveSeeking);
		fdc->makeAvailable();
		fdc->setSeekStatusOn(floppyDriveSeeking);
	}
	else
	{
		//TODO: set NR flag in status register 0 and quit
	}
}

void E5150::FDC::COMMAND::Seek::execOnFloppyDrive(Floppy100& drive) const
{
	drive.performeCommand<Floppy100::COMMAND::STEP>((unsigned int)m_configurationWords[2]);
}

//TODO: what happens when SRT timer isn't well configured
void E5150::FDC::COMMAND::Seek::exec()
{
	for (unsigned int driveNumber = 0; driveNumber < 4; ++driveNumber)
	{
		if ((1 << driveNumber) & m_floppyToApply)
		{
			//TODO: what to do if there is no floppy drive ?
			execOnFloppyDrive(fdc->m_floppyDrives[driveNumber]);
		}
	}
}

//////////////////////////////
/*** IMPLEMENTING INVALID ***/
//////////////////////////////

E5150::FDC::COMMAND::Invalid::Invalid(): Command("Invalid",1,1)
{ m_resultWords[0] = 0x80;   m_checkMFM = false;   m_saveHDS_DSx = false;}

void E5150::FDC::COMMAND::Invalid::onConfigureFinish() { fdc->switchToResultMode(); }