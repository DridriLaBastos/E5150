#include "fdc.hpp"

#define FDCDebug(CURRENT_DEBUG_LEVEL,...) debug<CURRENT_DEBUG_LEVEL>("FDC: " __VA_ARGS__)

//mmmh...
static E5150::FDC* fdc = nullptr;

//The IBM PC doc says that the floppy driver adapter have I/O port from 0x3F0 to 0x3F7 which means a 3 lines
//address bus is used. But there is only 3 registers for this adaptater: the DOR at 0x3F2 and the register
//from the UPD365 at 0x3F4, 0x3F5. The registers are maped like that:
//0b00x --> nothing
//0b010 --> DOR
//0b10x --> FDC
E5150::FDC::FDC(E5150::PIC& pic, PORTS& ports):
	Component("Floppy Controller",ports,0x3F0,0b111), m_pic(pic),m_statusRegister(0),m_dorRegister(0)
{
	fdc = this;

	m_commands[0]  = &invalid;          m_commands[1]  = &scanEqual;      m_commands[2]  = &readATrack;
	m_commands[3]  = &specify;          m_commands[4]  = &senseDriveStat; m_commands[5]  = &writeData;
	m_commands[6]  = &readData;         m_commands[7]  = &recalibrate;    m_commands[8]  = &senseInterruptStatus;
	m_commands[9]  = &writeDeletedData; m_commands[10] = &readID;         m_commands[11] = &scanLEQ;
	m_commands[12] = &readDeletedData;  m_commands[13] = &formatTrack;    m_commands[14] = &scanHEQ;
	m_commands[15] = &seek;

	reinit();
}

//TODO: search more info of the init state of the status register. For now is is set to the status:
// + all drives in seek mode
void E5150::FDC::reinit()
{
	m_dorRegister = 0;
	m_dorRegister |= DOR_REGISTER::FDC_RESET | DOR_REGISTER::IO;
	m_dataRegister = 0;
	m_passClock = 0;
	m_statusRegisterRead = false;

	m_timers[0] = 0;
	m_timers[1] = 0;
	m_timers[2] = 0;

	switchToCommandMode();
}

void E5150::FDC::waitClock (const unsigned int clock) { m_passClock += clock; debug<DEBUG_LEVEL_MAX>("FDC will wait {} clock(s)",m_passClock); }
void E5150::FDC::waitMicro (const unsigned int microseconds) { waitClock(microseconds*8); }
void E5150::FDC::waitMilli (const unsigned int milliseconds) { waitMicro(milliseconds*1000); }

void E5150::FDC::makeBusy () { m_statusRegister |= (1 << 4); }
void E5150::FDC::makeAvailable () { m_statusRegister &= ~(1 << 4); }

void E5150::FDC::setSeekStatusOn (const unsigned int driveNumber) { m_statusRegister |= (1 << driveNumber); }
void E5150::FDC::resetSeekStatusOf (const unsigned int driveNumber) { m_statusRegister &= ~(1 << driveNumber); }

void E5150::FDC::makeDataRegisterReady (void) { m_statusRegister |= (1 << 7); }
void E5150::FDC::makeDataRegisterNotReady (void) { m_statusRegister &= ~(1 << 7); }
void E5150::FDC::makeDataRegisterInReadMode (void) { m_statusRegister |= 1 << 6; }
void E5150::FDC::makeDataRegisterInWriteMode (void) { m_statusRegister &= ~(1 << 6); }

bool E5150::FDC::dataRegisterReady (void) const { return m_statusRegister & (1 << 7); }
bool E5150::FDC::dataRegisterInReadMode (void) const { return m_statusRegister & (1 << 6); }
bool E5150::FDC::dataRegisterInWriteMode (void) const { return !dataRegisterInReadMode(); }
bool E5150::FDC::statusRegisterAllowReading (void) const { return dataRegisterReady() && dataRegisterInReadMode() && m_statusRegisterRead; }
bool E5150::FDC::statusRegisterAllowWriting (void) const { return dataRegisterReady() && dataRegisterInWriteMode() && m_statusRegisterRead; }
bool E5150::FDC::isBusy (void) const { return m_statusRegister & 0b10000; }

void E5150::FDC::setST0Flag (const ST0_FLAGS flag)
{ m_STRegisters[0] |= flag; }
void E5150::FDC::resetST0Flag (const ST0_FLAGS flag)
{ m_STRegisters[0] &= ~flag; }

void E5150::FDC::clock()
{
	if (m_passClock == 0)
	{
		if (m_phase == PHASE::EXECUTION)
			m_commands[m_selectedCommand]->exec();
	}
	else
		--m_passClock;
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
	FDCDebug(7,"switched to command mode");
}

void E5150::FDC::switchToExecutionMode (void)
{
	m_phase = PHASE::EXECUTION;
	makeDataRegisterNotReady();
	FDCDebug(7,"switched to execution mode");
}

void E5150::FDC::switchToResultMode (void)
{
	m_phase = PHASE::RESULT;
	makeDataRegisterReady();
	makeDataRegisterInReadMode();
	FDCDebug(7,"switched to result mode");
}

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
		
		m_selectedCommand = commandIndex;
		
		FDCDebug(5,"Selecting command '{}'",m_commands[m_selectedCommand]->m_name);
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
					FDCDebug(1,"Data register {}", dataRegisterReady() ? "not in write mode" : "not ready");
			}
			else
				FDCDebug(1,"Status register not read before writing to data register");
		}

		default:
			FDCDebug(1,"Cannot write at address {:b}. Address should be 10b(0x3F2) for DOR or 100b (0x3F5) for data register",localAddress);
	}

	FDCDebug(1,"Writing not done");
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
					FDCDebug(1,"Data regisrer {}.",dataRegisterReady() ? "in write mode" : "not ready");
			}
			else
				FDCDebug(1,"Status register not read before reading data register");
		} break;
		
		default:
			FDCDebug(1,"Cannot read address {:b}. Address should be 10b (0x3F2) for DOR, 100b (0x3F4) or 101b (0x3F5) for status/data register",localAddress);
	}

	FDCDebug(1,"Value outputed will be undetermined");
	uint8_t undetermined;
	return 	undetermined;
}

///////////////////////////////////
/*** IMPLEMENTING THE COMMANDS ***/
///////////////////////////////////

static unsigned int getHDS (const std::vector<uint8_t>& configurationWords) { return (configurationWords[1] & 0b100) >> 2; }
static unsigned int getDSx (const std::vector<uint8_t>& configurationWords) { return configurationWords[1] & 0b11; }
static unsigned int getHDS (const unsigned int configurationWord) { return (configurationWord & 0b100) >> 2; }
static unsigned int getDSx (const unsigned int configurationWord) { return configurationWord & 0b11; }

E5150::FDC::Command::Command(const std::string& name, const unsigned int configurationWorldNumber, const unsigned int resultWorldNumber):
	m_name(name), m_configurationWords(configurationWorldNumber), m_resultWords(resultWorldNumber),m_configurationStep(0)
{}

bool E5150::FDC::Command::configure (const uint8_t data)
{
	//TODO: what to do here ? Probably not exiting the simulation
	if (fdc->isBusy())
	{
		FDCDebug(1,"New command issued while another command is being processed. Nothing done");
		return false;
	}

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
		m_configurationStep = 0;

		fdc->switchToExecutionMode();
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

void E5150::FDC::Command::onConfigureBegin()  {}
void E5150::FDC::Command::onConfigureFinish() {}
void E5150::FDC::Command::exec() {}

///////////////////////////////////
/*** IMPLEMENTING READ DATA ***/
///////////////////////////////////

void E5150::FDC::COMMAND::ReadData::loadHeads()
{ m_status = STATUS::READ_DATA; }

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
{ m_checkMFM = false; }

//////////////////////////////
/*** IMPLEMENTING SENSE INTERRUPT STATUS ***/
//////////////////////////////
E5150::FDC::COMMAND::SenseInterruptStatus::SenseInterruptStatus(): Command("Sense Interrupt Status",1,2)
{ m_checkMFM = false; }

void E5150::FDC::COMMAND::SenseInterruptStatus::onConfigureFinish()
{
	m_resultWords[0] = fdc->m_STRegisters[0];
	m_resultWords[1] = fdc->m_floppyDrives[getDSx(m_resultWords[0])].m_pcn;
	fdc->switchToResultMode();
}
//////////////////////////////
/*** IMPLEMENTING SPECIFY ***/
//////////////////////////////

E5150::FDC::COMMAND::Specify::Specify(): Command("Specify",3,0)
{ m_checkMFM = false; }

//*2 on all result because the clock is at 4MHz
//TODO: fact checking this
static unsigned int millisecondsFromSRTTimer (const unsigned int SRTValue) { return (0xF - SRTValue + 1); }
static unsigned int millisecondsFromHUTTimer (const unsigned int HUTValue) { return HUTValue * 16; }
static unsigned int millisecondsFromHLTTimer (const unsigned int HLTValue) { return HLTValue * 2; }

void E5150::FDC::COMMAND::Specify::onConfigureFinish()
{
	const uint8_t SRTValue = m_configurationWords[1] >> 4;
	const uint8_t HUTValue = m_configurationWords[1] & 0xF;
	const uint8_t HLTValue = m_configurationWords[2] >> 1;

	if (HUTValue == 0)
		throw std::logic_error("HUT cannot be 0");
	
	if ((HLTValue == 0) || (HLTValue == 0xFF))
		throw std::logic_error("HLT value should be in [0x1, 0xFE]");

	fdc->m_timers[TIMER::STEP_RATE_TIME] = SRTValue;
	fdc->m_timers[TIMER::HEAD_UNLOAD_TIME] = HUTValue;
	fdc->m_timers[TIMER::HEAD_LOAD_TIME] = HLTValue;

	const unsigned int SRTTimerMSValue = millisecondsFromSRTTimer(fdc->m_timers[TIMER::STEP_RATE_TIME]);
	const unsigned int HUTTimerMSValue = millisecondsFromHUTTimer(fdc->m_timers[TIMER::HEAD_UNLOAD_TIME]);
	const unsigned int HLTTimerMSValue = millisecondsFromHLTTimer(fdc->m_timers[TIMER::HEAD_LOAD_TIME]);

	FDCDebug(1,"SRT Value set to {}ms",SRTTimerMSValue*2);
	FDCDebug(1,"HUT Value set to {}ms",HUTTimerMSValue*2);
	FDCDebug(1,"HLT Value set to {}ms",HLTTimerMSValue*2);
	
	fdc->switchToCommandMode();
	//TODO: investigate timing : I assume one write per clock, it might be more or less
	//fdc->waitClock(3);
}

//////////////////////////////
/*** IMPLEMENTING    SEEK ***/
//////////////////////////////

E5150::FDC::COMMAND::Seek::Seek(): Command("Seek",3,0) { m_checkMFM = false; }

void E5150::FDC::COMMAND::Seek::onConfigureBegin ()
{
	fdc->makeBusy();
	m_firstStep = true; //The next step issued will be the first of the command
}

void E5150::FDC::COMMAND::Seek::onConfigureFinish()
{
	const unsigned int floppyIndex = m_configurationWords[1] & 0b11;
	m_floppyToApply = &fdc->m_floppyDrives[floppyIndex];

	const unsigned int pcn = m_floppyToApply->m_pcn;
	const unsigned int ncn = m_configurationWords[2];
	m_direction = ncn > pcn;

	fdc->setSeekStatusOn(m_floppyToApply->driverNumber);
}

void E5150::FDC::COMMAND::Seek::finish(const unsigned int endFlags)
{
	const unsigned int st0Flags = endFlags | getDSx(m_configurationWords);
	fdc->m_STRegisters[0] = st0Flags;
	fdc->resetSeekStatusOf(m_floppyToApply->driverNumber);
	fdc->switchToCommandMode();
	//TODO: this shouldn't be there, but for now I don't know how to make multiple seek at a time
	fdc->makeAvailable();
	fdc->m_pic.assertInteruptLine(PIC::IR6);
}

//TODO: what happens when SRT timer isn't well configured
//TODO: how multiple seeks work ?
void E5150::FDC::COMMAND::Seek::exec()
{
	if (!m_floppyToApply->isReady())
	{
		FDCDebug(5,"Floppy {} not ready", m_floppyToApply->driverNumber);
		FDCDebug(6,"SEEK COMMAND: Termination with ready line state change");
		fdc->setST0Flag(ST0_FLAGS::NR);
		finish(ST0_FLAGS::IC1 | ST0_FLAGS::IC2);
		return;
	}

	if (m_floppyToApply->m_pcn != m_configurationWords[2])
	{
		//The time waited will be multiplied by 2 because the function returns the milliseconds value for a 8MHz clock
		//but the clock of the fdc is a 4MHz one
		const unsigned int millisecondsValueFromSRTTimer = millisecondsFromSRTTimer(fdc->m_timers[TIMER::STEP_RATE_TIME]);
		const Milliseconds millisecondsToWait (millisecondsValueFromSRTTimer*2);

		const bool stepSuccess = m_floppyToApply->step(m_direction,millisecondsToWait,m_firstStep);

		if (!stepSuccess)
		{
			FDCDebug(6,"SEEK COMMAND: Abnormal termination");
			finish(ST0_FLAGS::SE | ST0_FLAGS::IC1);
		}
		else
			fdc->waitMilli(millisecondsValueFromSRTTimer);

	}
	else
		finish(ST0_FLAGS::SE);

	m_firstStep = false;
}

//////////////////////////////
/*** IMPLEMENTING INVALID ***/
//////////////////////////////

E5150::FDC::COMMAND::Invalid::Invalid(): Command("Invalid",1,1)
{ m_resultWords[0] = 0x80;   m_checkMFM = false; }

void E5150::FDC::COMMAND::Invalid::onConfigureFinish() { fdc->switchToResultMode(); }