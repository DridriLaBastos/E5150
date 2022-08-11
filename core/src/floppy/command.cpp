#include "core/command.hpp"

/**
 * Maybe all this template stuffs are not needed but I wanted to try something because I don't template
 * programming often, I wanted to try and learn. The idea was to reflect the fact that we do know at compile
 * time what a command does
 */
static constexpr bool CHECK_MFM=true;
static constexpr bool DONT_CHECK_MFM=false;

static constexpr bool SET_FDD_HEAD=true;
static constexpr bool DONT_SET_FDD_HEAD=false;

static constexpr unsigned int COMMAND_MODE_AFTER_CONFIGURE = 0;
static constexpr unsigned int EXEC_MODE_AFTER_CONFIGURE = 1;
static constexpr unsigned int RESULT_MODE_AFTER_CONFIGURE = 2;

template <bool CHECK_MFM_T,bool SET_FDD_HEAD_T=true,unsigned int MODE_AFTER_CONFIGURE=EXEC_MODE_AFTER_CONFIGURE>
static void generalConfigurationEnd (void)
{
	if constexpr (CHECK_MFM_T)
	{
		if (!(E5150::FDC::instance->configurationDatas[0] & (1 << 6)))
			throw std::logic_error("FM mode is not supported with the floppy drive");
	}

	if constexpr (SET_FDD_HEAD_T)
	{
		const unsigned int FDDNumber = E5150::FDC::instance->configurationDatas[1] & 0b11;
		const unsigned int headdAddress = (E5150::FDC::instance->configurationDatas[1] & 0b100) >> 2;
		E5150::FDC::instance->floppyDrives[FDDNumber].setHeadAddress(headdAddress);
	}

	if constexpr (MODE_AFTER_CONFIGURE == COMMAND_MODE_AFTER_CONFIGURE)
		E5150::FDC::instance->switchToCommandMode();
	
	if constexpr (MODE_AFTER_CONFIGURE == EXEC_MODE_AFTER_CONFIGURE)
		E5150::FDC::instance->switchToExecutionMode();
	
	if constexpr (MODE_AFTER_CONFIGURE == RESULT_MODE_AFTER_CONFIGURE)
		E5150::FDC::instance->switchToResultMode();
}

/*** BASE CLASS ***/
E5150::FDC_COMMAND::Command::Command(const std::string& name,const unsigned int configNumber,const unsigned int resultNumber):
m_name(name),configurationWordsNumber(configNumber),resultWordsNumber(resultNumber)
{}

void E5150::FDC_COMMAND::Command::configurationBegin()  {}
void E5150::FDC_COMMAND::Command::configurationEnd() { generalConfigurationEnd<CHECK_MFM,SET_FDD_HEAD,EXEC_MODE_AFTER_CONFIGURE>(); }
void E5150::FDC_COMMAND::Command::exec() {}

static unsigned int millisecondsFromSRTTimer (void) { return (0xF - E5150::FDC::instance->timers[E5150::FDC::STEP_RATE_TIME] + 1); }
static unsigned int millisecondsFromHUTTimer (void) { return E5150::FDC::instance->timers[E5150::FDC::HEAD_UNLOAD_TIME] * 16; }
static unsigned int millisecondsFromHLTTimer (void) { return E5150::FDC::instance->timers[E5150::FDC::HEAD_LOAD_TIME] * 2; }

/******************************************************************************************/
/*                                     * READ  DATA *                                     */
/******************************************************************************************/
void E5150::FDC_COMMAND::ReadData::configurationEnd()
{
	loadHeadRequested=false;
	E5150::FDC::instance->readCommandInProcess = true;
	E5150::FDC::instance->makeDataRegisterInReadMode();

	if ((E5150::FDC::instance->configurationDatas[5] != 0) && (E5150::FDC::instance->configurationDatas[8] != 0x0F))
		FDCEmulationLog(EMULATION_MAX_LOG_LEVEL," READ DATA: DTL (configuration word 9) should be 0xFF when N (configuration word 6) is non 0");
}

void E5150::FDC_COMMAND::ReadData::finish(const unsigned int st1Flags)
{
	E5150::FDC::instance->STRegisters[1] = st1Flags;
	E5150::FDC::instance->interruptPIC();
	E5150::FDC::instance->switchToResultMode();
}

//TODO:
// - implement DMA transfert
// - CRC check (really ?)
// - DTL to 0x0FF when N is non 0
// - detection of index hole
// - implement getting sector id
//From IBM doc: 250K bits/sec --> 32000o/s --> 31.25 us/octet --> 125 clocks at 4MHz (250 ns/clk)
void E5150::FDC_COMMAND::ReadData::exec()
{
	if (!floppyToApply->headLoaded() && !loadHeadRequested)
	{
		floppyToApply->loadHeads();
		loadHeadRequested = false;
		E5150::FDC::instance->waitMilli(millisecondsFromHLTTimer());
		return;
	}

	//TODO: How exact this counter should be ?
	if (E5150::FDC::instance->clockFromCommandStart != 52)
	{
		finish(E5150::FDC::ST1_FLAGS::OR);
		return;
	}

	//TODO:
	//if terminalCount
	//	stop uploading data
	//	continue reading until end of sector
	//	terminate count
	const uint8_t readFromFloppy = floppyToApply->read();

	//if N==0
	if (E5150::FDC::instance->configurationDatas[5] == 0)
	{
		if (floppyToApply->currentID.number > E5150::FDC::instance->configurationDatas[8])
			return;
	}

	E5150::FDC::instance->makeDataRegisterReady();
	E5150::FDC::instance->readFromFloppy = floppyToApply->read();
	E5150::FDC::instance->picConnected.assertInterruptLine(E5150::FDC::ConnectedIRLine,E5150::FDC::instance);
	return;

	E5150::FDC::instance->clockFromCommandStart = 0;
}

/******************************************************************************************/
/*                                      * READ  ID *                                      */
/******************************************************************************************/
E5150::FDC_COMMAND::ReadID::ReadID(): Command("Read ID",2) {}

//TODO: timing !
void E5150::FDC_COMMAND::ReadID::exec()
{
	const unsigned int selectedHead = (E5150::FDC::instance->configurationDatas[1] & 0b100) >> 2;
	const auto& [c,h,r,n] = E5150::FDC::instance->floppyDrives[selectedHead].getID();

	E5150::FDC::instance->resultDatas[0] = E5150::FDC::instance->STRegisters[0];
	E5150::FDC::instance->resultDatas[1] = E5150::FDC::instance->STRegisters[1];
	E5150::FDC::instance->resultDatas[2] = E5150::FDC::instance->STRegisters[2];
	E5150::FDC::instance->resultDatas[3] = c;
	E5150::FDC::instance->resultDatas[4] = h;
	E5150::FDC::instance->resultDatas[5] = r;
	E5150::FDC::instance->resultDatas[6] = n;

	//TODO: investigate timing : I assume one write per clock, it might be more or less
	//In bochs this took 11 111 clocks
	E5150::FDC::instance->waitClock(7);
	E5150::FDC::instance->switchToResultMode();
}

/******************************************************************************************/
/*                                     * RECALIBRATE *                                    */
/******************************************************************************************/
E5150::FDC_COMMAND::Recalibrate::Recalibrate(): Command("Recalibrate",2,0){}

void E5150::FDC_COMMAND::Recalibrate::configurationBegin()
{
	E5150::FDC::instance->makeBusy();
	m_firstStep = true; //The next step issued will be the first of the command
}

void E5150::FDC_COMMAND::Recalibrate::configurationEnd()
{
	generalConfigurationEnd<DONT_CHECK_MFM,DONT_SET_FDD_HEAD>();
	const unsigned int floppyIndex = E5150::FDC::instance->configurationDatas[1] & 0b11;
	floppyToApply = &E5150::FDC::instance->floppyDrives[floppyIndex];
	E5150::FDC::instance->setSeekStatusOn(floppyIndex);
}

void E5150::FDC_COMMAND::Recalibrate::finish (const unsigned int endFlags)
{
	const unsigned int st0Flags = endFlags | (E5150::FDC::instance->configurationDatas[1] & 0b111);
	E5150::FDC::instance->STRegisters[0] = st0Flags;
	E5150::FDC::instance->resetSeekStatusOf(floppyToApply->driverNumber);
	//TODO: this shouldn't be there, but for now I don't know how to make multiple seek at a time
	E5150::FDC::instance->makeAvailable();
	E5150::FDC::instance->picConnected.assertInterruptLine(PIC::IR6,E5150::FDC::instance);
	E5150::FDC::instance->switchToCommandMode();
}

void E5150::FDC_COMMAND::Recalibrate::exec()
{
	static unsigned int stepCount = 0;

	if (m_firstStep)
		stepCount = 0;

	if (!floppyToApply->isReady())
	{
		FDCEmulationLog(5,"Floppy {} not ready", floppyToApply->driverNumber);
		FDCEmulationLog(6,"RECALIBRATE COMMAND: Termination with ready line state change");
		finish(E5150::FDC::ST0_FLAGS::NR | E5150::FDC::ST0_FLAGS::IC1 | E5150::FDC::ST0_FLAGS::IC2);
		return;
	}

	if (floppyToApply->currentID.cylinder == 0)
	{
		finish(E5150::FDC::ST0_FLAGS::SE);
		return;
	}
	
	if (stepCount == 77)
	{
		FDCEmulationLog(4,"RECALIBRATE COMMAND: Termination with 77 step issued without track0 found");
		finish(E5150::FDC::ST0_FLAGS::SE | E5150::FDC::ST0_FLAGS::EC);
	}

	const unsigned int millisecondsValueFromSRTTimer = millisecondsFromSRTTimer();
	const Milliseconds millisecondsToWait (millisecondsValueFromSRTTimer*2);

	const bool stepSuccess = floppyToApply->step(false,millisecondsToWait,m_firstStep);

	if (!stepSuccess)
	{
		FDCEmulationLog(6,"RECALIBRATE COMMAND: Abnormal termination");
		finish(E5150::FDC::ST0_FLAGS::SE | E5150::FDC::ST0_FLAGS::IC1);
	}
	else
		E5150::FDC::instance->waitMilli(millisecondsValueFromSRTTimer);

	m_firstStep = false;
	++stepCount;
}

/*******************************************************************************************/
/*                                * SENSE INTERRUPT STATUS *                               */
/*******************************************************************************************/
E5150::FDC_COMMAND::SenseInterruptStatus::SenseInterruptStatus(): Command("Sense Interrupt Status",1,2){}

void E5150::FDC_COMMAND::SenseInterruptStatus::configurationEnd()
{
	generalConfigurationEnd<DONT_CHECK_MFM,DONT_SET_FDD_HEAD,RESULT_MODE_AFTER_CONFIGURE>();
	E5150::FDC::instance->resultDatas[0] = E5150::FDC::instance->STRegisters[0];
	E5150::FDC::instance->resultDatas[1] = E5150::FDC::instance->floppyDrives[E5150::FDC::instance->configurationDatas[1]&0b11].currentID.cylinder;
}

/*******************************************************************************************/
/*                                       * SPECIFY *                                       */
/*******************************************************************************************/
E5150::FDC_COMMAND::Specify::Specify(): Command("Specify",3,0) {}

void E5150::FDC_COMMAND::Specify::configurationEnd()
{
	const uint8_t SRTValue = E5150::FDC::instance->configurationDatas[1] >> 4;
	const uint8_t HUTValue = E5150::FDC::instance->configurationDatas[1] & 0xF;
	const uint8_t HLTValue = E5150::FDC::instance->configurationDatas[2] >> 1;

	if (HUTValue == 0)
		throw std::logic_error("HUT cannot be 0");
	
	if ((HLTValue == 0) || (HLTValue == 0xFF))
		throw std::logic_error("HLT value should be in [0x1, 0xFE]");

	const unsigned int SRTTimerMSValue = millisecondsFromSRTTimer();
	const unsigned int HUTTimerMSValue = millisecondsFromHUTTimer();
	const unsigned int HLTTimerMSValue = millisecondsFromHLTTimer();

	FDCEmulationLog(1,"SRT Value set to {}ms",SRTTimerMSValue*2);
	FDCEmulationLog(1,"HUT Value set to {}ms",HUTTimerMSValue*2);
	FDCEmulationLog(1,"HLT Value set to {}ms",HLTTimerMSValue*2);
	generalConfigurationEnd<DONT_CHECK_MFM,DONT_SET_FDD_HEAD,COMMAND_MODE_AFTER_CONFIGURE>();
}

/******************************************************************************************/
/*                                 * SENSE DRIVE STATUS *                                 */
/******************************************************************************************/
E5150::FDC_COMMAND::SenseDriveStatus::SenseDriveStatus(): Command("Sense Drive Status",2,1){}

void E5150::FDC_COMMAND::SenseDriveStatus::configurationEnd()
{
	const unsigned int floppyIndex = E5150::FDC::instance->configurationDatas[2] & 0b11;
	E5150::FDC::instance->resultDatas[0] = E5150::FDC::instance->floppyDrives[floppyIndex].getStatusRegister3();
	generalConfigurationEnd<DONT_CHECK_MFM,SET_FDD_HEAD,RESULT_MODE_AFTER_CONFIGURE>();
}


/******************************************************************************************/
/*                                        * SEEK *                                        */
/******************************************************************************************/
E5150::FDC_COMMAND::Seek::Seek(): Command("Seek",3,0) {}

void E5150::FDC_COMMAND::Seek::configurationBegin ()
{
	E5150::FDC::instance->makeBusy();
	m_firstStep = true; //The next step issued will be the first of the command
}

void E5150::FDC_COMMAND::Seek::configurationEnd()
{
	const unsigned int floppyIndex = E5150::FDC::instance->configurationDatas[1] & 0b11;
	floppyToApply = &E5150::FDC::instance->floppyDrives[floppyIndex];

	const unsigned int ncn = E5150::FDC::instance->configurationDatas[2];
	m_direction = ncn > floppyToApply->currentID.cylinder;

	E5150::FDC::instance->setSeekStatusOn(floppyIndex);
	generalConfigurationEnd<DONT_CHECK_MFM>();
}

void E5150::FDC_COMMAND::Seek::finish(const unsigned int st0Flags, const unsigned int st1Flags = 0)
{
	E5150::FDC::instance->STRegisters[0] = st0Flags | (E5150::FDC::instance->configurationDatas[1] & 0b111);
	E5150::FDC::instance->STRegisters[1] = st1Flags;
	E5150::FDC::instance->resetSeekStatusOf(floppyToApply->driverNumber);
	//TODO: this shouldn't be there, but for now I don't know how to make multiple seek at a time
	E5150::FDC::instance->makeAvailable();
	E5150::FDC::instance->interruptPIC();
	E5150::FDC::instance->switchToCommandMode();
}

//TODO: how multiple seeks work ?
void E5150::FDC_COMMAND::Seek::exec()
{
	if (!floppyToApply->isReady())
	{
		FDCEmulationLog(5,"Floppy {} not ready", floppyToApply->driverNumber);
		FDCEmulationLog(6,"SEEK COMMAND: Termination with ready line state change");
		finish(E5150::FDC::ST0_FLAGS::NR | E5150::FDC::ST0_FLAGS::IC1 | E5150::FDC::ST0_FLAGS::IC2);
		return;
	}

	if (floppyToApply->currentID.cylinder == E5150::FDC::instance->configurationDatas[2])
	{
		finish(E5150::FDC::ST0_FLAGS::SE);
		return;
	}

	
	//The time waited will be multiplied by 2 because the function returns the milliseconds value for a 8MHz clock
	//but the clock of the E5150::fdc is a 4MHz one in the 5150
	const unsigned int millisecondsValueFromSRTTimer = millisecondsFromSRTTimer();
	const Milliseconds millisecondsToWait (millisecondsValueFromSRTTimer*2);

	const bool stepSuccess = floppyToApply->step(m_direction,millisecondsToWait,m_firstStep);

	if (!stepSuccess)
	{
		FDCEmulationLog(6,"SEEK COMMAND: Abnormal termination");
		finish(E5150::FDC::ST0_FLAGS::SE | E5150::FDC::ST0_FLAGS::IC1);
	}
	else
		E5150::FDC::instance->waitMilli(millisecondsValueFromSRTTimer);


	m_firstStep = false;
}

/*** IMPLEMENTING INVALID ***/

E5150::FDC_COMMAND::Invalid::Invalid(): Command("Invalid",1,1)
{}

void E5150::FDC_COMMAND::Invalid::configurationEnd()
{
	E5150::FDC::instance->resultDatas[0]=0x80;
	generalConfigurationEnd<DONT_CHECK_MFM,DONT_SET_FDD_HEAD,RESULT_MODE_AFTER_CONFIGURE>();
}