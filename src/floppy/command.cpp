#include "command.hpp"

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
		if (!(FDC::instance->configurationDatas[0] & (1 << 6)))
			throw std::logic_error("FM mode is not supported with the floppy drive");
	}

	if constexpr (SET_FDD_HEAD_T)
	{
		const unsigned int FDDNumber = FDC::instance->configurationDatas[1] & 0b11;
		const unsigned int headdAddress = (FDC::instance->configurationDatas[1] & 0b100) >> 2;
		FDC::instance->floppyDrives[FDDNumber].setHeadAddress(headdAddress);
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

///////////////////////////////////
/***         READ DATA         ***/
///////////////////////////////////

void E5150::FDC_COMMAND::ReadData::loadHeads()
{ m_status = STATUS::READ_DATA; }

void E5150::FDC_COMMAND::ReadData::exec()
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

/******************************************************************************************/
/*                                      * READ  ID *                                      */
/******************************************************************************************/
E5150::FDC_COMMAND::ReadID::ReadID(): Command("Read ID",2) {}

//TODO: timing !
void E5150::FDC_COMMAND::ReadID::exec()
{
	const unsigned int selectedHead = (FDC::instance->configurationDatas[1] & 0b100) >> 2;
	const auto& [c,h,r,n] = FDC::instance->floppyDrives[selectedHead].getID();

	FDC::instance->resultDatas[0] = FDC::instance->STRegisters[0];
	FDC::instance->resultDatas[1] = FDC::instance->STRegisters[1];
	FDC::instance->resultDatas[2] = FDC::instance->STRegisters[2];
	FDC::instance->resultDatas[3] = c;
	FDC::instance->resultDatas[4] = h;
	FDC::instance->resultDatas[5] = r;
	FDC::instance->resultDatas[6] = n;

	//TODO: investigate timing : I assume one write per clock, it might be more or less
	//In bochs this took 11 111 clocks
	FDC::instance->waitClock(7);
	FDC::instance->switchToResultMode();
}

/******************************************************************************************/
/*                                     * RECALIBRATE *                                    */
/******************************************************************************************/
E5150::FDC_COMMAND::Recalibrate::Recalibrate(): Command("Recalibrate",2,0){}

void E5150::FDC_COMMAND::Recalibrate::configurationBegin()
{
	FDC::instance->makeBusy();
	m_firstStep = true; //The next step issued will be the first of the command
}

void E5150::FDC_COMMAND::Recalibrate::configurationEnd()
{
	generalConfigurationEnd<DONT_CHECK_MFM,DONT_SET_FDD_HEAD>();
	const unsigned int floppyIndex = FDC::instance->configurationDatas[1] & 0b11;
	m_floppyToApply = &FDC::instance->floppyDrives[floppyIndex];
	FDC::instance->setSeekStatusOn(floppyIndex);
}

void E5150::FDC_COMMAND::Recalibrate::finish (const unsigned int endFlags)
{
	const unsigned int st0Flags = endFlags | (FDC::instance->configurationDatas[1] & 0b111);
	FDC::instance->STRegisters[0] = st0Flags;
	FDC::instance->resetSeekStatusOf(m_floppyToApply->driverNumber);
	//TODO: this shouldn't be there, but for now I don't know how to make multiple seek at a time
	FDC::instance->makeAvailable();
	FDC::instance->picConnected.assertInterruptLine(PIC::IR6,FDC::instance);
	FDC::instance->switchToCommandMode();
}

static unsigned int millisecondsFromSRTTimer (const unsigned int SRTValue) { return (0xF - SRTValue + 1); }
static unsigned int millisecondsFromHUTTimer (const unsigned int HUTValue) { return HUTValue * 16; }
static unsigned int millisecondsFromHLTTimer (const unsigned int HLTValue) { return HLTValue * 2; }

//The step count behaviour may never be implemented because we are not on real hardware.
//But it is here in case of I found a way to implement it
void E5150::FDC_COMMAND::Recalibrate::exec()
{
	static unsigned int stepCount = 0;

	if (m_firstStep)
		stepCount = 0;

	if (!m_floppyToApply->isReady())
	{
		FDCDebug(5,"Floppy {} not ready", m_floppyToApply->driverNumber);
		FDCDebug(6,"RECALIBRATE COMMAND: Termination with ready line state change");
		finish(FDC::ST0_FLAGS::NR | FDC::ST0_FLAGS::IC1 | FDC::ST0_FLAGS::IC2);
		return;
	}

	if (m_floppyToApply->pcn == 0)
	{
		finish(FDC::ST0_FLAGS::SE);
		return;
	}
	
	if (stepCount == 77)
	{
		FDCDebug(4,"RECALIBRATE COMMAND: Termination with 77 step issued without track0 found");
		finish(FDC::ST0_FLAGS::SE | FDC::ST0_FLAGS::EC);
	}

	const unsigned int millisecondsValueFromSRTTimer = millisecondsFromSRTTimer(FDC::instance->timers[FDC::TIMER::STEP_RATE_TIME]);
	const Milliseconds millisecondsToWait (millisecondsValueFromSRTTimer*2);

	const bool stepSuccess = m_floppyToApply->step(false,millisecondsToWait,m_firstStep);

	if (!stepSuccess)
	{
		FDCDebug(6,"RECALIBRATE COMMAND: Abnormal termination");
		finish(FDC::ST0_FLAGS::SE | FDC::ST0_FLAGS::IC1);
	}
	else
		FDC::instance->waitMilli(millisecondsValueFromSRTTimer);

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
	FDC::instance->resultDatas[0] = FDC::instance->STRegisters[0];
	FDC::instance->resultDatas[1] = FDC::instance->floppyDrives[FDC::instance->configurationDatas[1]&0b11].pcn;
}

/*******************************************************************************************/
/*                                       * SPECIFY *                                       */
/*******************************************************************************************/
E5150::FDC_COMMAND::Specify::Specify(): Command("Specify",3,0) {}

void E5150::FDC_COMMAND::Specify::configurationEnd()
{
	const uint8_t SRTValue = FDC::instance->configurationDatas[1] >> 4;
	const uint8_t HUTValue = FDC::instance->configurationDatas[1] & 0xF;
	const uint8_t HLTValue = FDC::instance->configurationDatas[2] >> 1;

	if (HUTValue == 0)
		throw std::logic_error("HUT cannot be 0");
	
	if ((HLTValue == 0) || (HLTValue == 0xFF))
		throw std::logic_error("HLT value should be in [0x1, 0xFE]");

	FDC::instance->timers[FDC::TIMER::STEP_RATE_TIME] = SRTValue;
	FDC::instance->timers[FDC::TIMER::HEAD_UNLOAD_TIME] = HUTValue;
	FDC::instance->timers[FDC::TIMER::HEAD_LOAD_TIME] = HLTValue;

	const unsigned int SRTTimerMSValue = millisecondsFromSRTTimer(FDC::instance->timers[FDC::TIMER::STEP_RATE_TIME]);
	const unsigned int HUTTimerMSValue = millisecondsFromHUTTimer(FDC::instance->timers[FDC::TIMER::HEAD_UNLOAD_TIME]);
	const unsigned int HLTTimerMSValue = millisecondsFromHLTTimer(FDC::instance->timers[FDC::TIMER::HEAD_LOAD_TIME]);

	FDCDebug(1,"SRT Value set to {}ms",SRTTimerMSValue*2);
	FDCDebug(1,"HUT Value set to {}ms",HUTTimerMSValue*2);
	FDCDebug(1,"HLT Value set to {}ms",HLTTimerMSValue*2);
	generalConfigurationEnd<DONT_CHECK_MFM,DONT_SET_FDD_HEAD,COMMAND_MODE_AFTER_CONFIGURE>();
}

/******************************************************************************************/
/*                                 * SENSE DRIVE STATUS *                                 */
/******************************************************************************************/
E5150::FDC_COMMAND::SenseDriveStatus::SenseDriveStatus(): Command("Sense Drive Status",2,1){}

void E5150::FDC_COMMAND::SenseDriveStatus::configurationEnd()
{
	const unsigned int floppyIndex = FDC::instance->configurationDatas[2] & 0b11;
	FDC::instance->resultDatas[0] = FDC::instance->floppyDrives[floppyIndex].getStatusRegister3();
	generalConfigurationEnd<DONT_CHECK_MFM,SET_FDD_HEAD,RESULT_MODE_AFTER_CONFIGURE>();
}


/******************************************************************************************/
/*                                        * SEEK *                                        */
/******************************************************************************************/
E5150::FDC_COMMAND::Seek::Seek(): Command("Seek",3,0) {}

void E5150::FDC_COMMAND::Seek::configurationBegin ()
{
	FDC::instance->makeBusy();
	m_firstStep = true; //The next step issued will be the first of the command
}

void E5150::FDC_COMMAND::Seek::configurationEnd()
{
	const unsigned int floppyIndex = FDC::instance->configurationDatas[1] & 0b11;
	m_floppyToApply = &FDC::instance->floppyDrives[floppyIndex];

	const unsigned int pcn = m_floppyToApply->pcn;
	const unsigned int ncn = FDC::instance->configurationDatas[2];
	m_direction = ncn > pcn;

	FDC::instance->setSeekStatusOn(floppyIndex);
	generalConfigurationEnd<DONT_CHECK_MFM>();
}

void E5150::FDC_COMMAND::Seek::finish(const unsigned int endFlags)
{
	const unsigned int st0Flags = endFlags | (FDC::instance->configurationDatas[1] & 0b111);
	FDC::instance->STRegisters[0] = st0Flags;
	FDC::instance->resetSeekStatusOf(m_floppyToApply->driverNumber);
	//TODO: this shouldn't be there, but for now I don't know how to make multiple seek at a time
	FDC::instance->makeAvailable();
	FDC::instance->picConnected.assertInterruptLine(PIC::IR6,FDC::instance);
	FDC::instance->switchToCommandMode();
}

//TODO: how multiple seeks work ?
void E5150::FDC_COMMAND::Seek::exec()
{
	if (!m_floppyToApply->isReady())
	{
		FDCDebug(5,"Floppy {} not ready", m_floppyToApply->driverNumber);
		FDCDebug(6,"SEEK COMMAND: Termination with ready line state change");
		finish(FDC::ST0_FLAGS::NR | FDC::ST0_FLAGS::IC1 | FDC::ST0_FLAGS::IC2);
		return;
	}

	if (m_floppyToApply->pcn == FDC::instance->configurationDatas[2])
	{
		finish(FDC::ST0_FLAGS::SE);
		return;
	}

	
	//The time waited will be multiplied by 2 because the function returns the milliseconds value for a 8MHz clock
	//but the clock of the fdc is a 4MHz one in the 5150
	const unsigned int millisecondsValueFromSRTTimer = millisecondsFromSRTTimer(FDC::instance->timers[FDC::TIMER::STEP_RATE_TIME]);
	const Milliseconds millisecondsToWait (millisecondsValueFromSRTTimer*2);

	const bool stepSuccess = m_floppyToApply->step(m_direction,millisecondsToWait,m_firstStep);

	if (!stepSuccess)
	{
		FDCDebug(6,"SEEK COMMAND: Abnormal termination");
		finish(FDC::ST0_FLAGS::SE | FDC::ST0_FLAGS::IC1);
	}
	else
		FDC::instance->waitMilli(millisecondsValueFromSRTTimer);


	m_firstStep = false;
}

/*** IMPLEMENTING INVALID ***/

E5150::FDC_COMMAND::Invalid::Invalid(): Command("Invalid",1,1)
{}

void E5150::FDC_COMMAND::Invalid::configurationEnd()
{
	FDC::instance->resultDatas[0]=0x80;
	generalConfigurationEnd<DONT_CHECK_MFM,DONT_SET_FDD_HEAD,RESULT_MODE_AFTER_CONFIGURE>();
}