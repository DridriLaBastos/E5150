#include "fdc.hpp"
#include "command.hpp"

using namespace E5150;

FDC* FDC::instance = nullptr;
FDC* fdc = nullptr;

static FDC_COMMAND::ReadData readData;
static FDC_COMMAND::ReadDeletedData readDeletedData;
static FDC_COMMAND::ReadATrack readATrack;
static FDC_COMMAND::ReadID readID;
static FDC_COMMAND::FormatTrack formatTrack;
static FDC_COMMAND::ScanEqual scanEqual;
static FDC_COMMAND::WriteData writeData;
static FDC_COMMAND::WriteDeletedData writeDeletedData;
static FDC_COMMAND::ScanLEQ scanLEQ;
static FDC_COMMAND::ScanHEQ scanHEQ;
static FDC_COMMAND::Recalibrate recalibrate;
static FDC_COMMAND::SenseInterruptStatus senseInterruptStatus;
static FDC_COMMAND::Specify specify;
static FDC_COMMAND::SenseDriveStatus senseDriveStat;
static FDC_COMMAND::Seek seek;
static FDC_COMMAND::Invalid invalid;

static std::array<FDC_COMMAND::Command*, 16> commands 
{
	&invalid,          &scanEqual,      &readATrack,
	&specify,          &senseDriveStat, &writeData,
	&readData,         &recalibrate,    &senseInterruptStatus,
	&writeDeletedData, &readID,         &scanLEQ,
	&readDeletedData,  &formatTrack,    &scanHEQ,
	&seek
};

static FDC_COMMAND::Command* selectedCommand = nullptr;

//TODO: search more info of the init state of the status register. For now it is set to the status:
// + all drives in seek mode
static void reinit()
{
	fdc->dorRegister = 0;
	fdc->dorRegister |= FDC::DOR_REGISTER::FDC_RESET | FDC::DOR_REGISTER::IO;
	fdc->dataRegister = 0;
	fdc->passClock = 0;
	fdc->clockFromCommandStart = 0;
	fdc->statusRegisterRead = false;
	fdc->readCommandInProcess = false;

	fdc->timers[0] = 0;
	fdc->timers[1] = 0;
	fdc->timers[2] = 0;

	fdc->readFromFloppy = 0;

	fdc->switchToCommandMode();
}

//The IBM PC doc says that the floppy driver adapter have I/O port from 0x3F0 to 0x3F7 which means a 3 lines
//address bus is used. But there is only 3 registers for this adaptater: the DOR at 0x3F2 and the register
//from the UPD365 at 0x3F4, 0x3F5. The registers are maped like that:
//0b00x --> nothing
//0b010 --> DOR
//0b10x --> FDC
E5150::FDC::FDC(E5150::PIC& pic, PORTS& ports):
	Component("Floppy Controller",ports,0x3F0,0b111), picConnected(pic),statusRegister(0),dorRegister(0)
{
	instance = this;
	fdc = this;

	reinit();
}

void E5150::FDC::makeDataRegisterReady () { fdc->statusRegister |= (1 << 7); }
void E5150::FDC::makeDataRegisterNotReady () { fdc->statusRegister &= ~(1 << 7); }
void E5150::FDC::makeDataRegisterInReadMode () { fdc->statusRegister |= 1 << 6; }
void E5150::FDC::makeDataRegisterInWriteMode () { fdc->statusRegister &= ~(1 << 6); }

void E5150::FDC::switchToCommandMode (void)
{
	phase = FDC::PHASE::COMMAND;
	selectedCommand = nullptr;
	makeDataRegisterReady();
	makeDataRegisterInWriteMode();
	FDCEmulationLog(7,"switched to command mode");
}

void E5150::FDC::switchToExecutionMode (void)
{
	phase = FDC::PHASE::EXECUTION;
	makeDataRegisterNotReady();
	FDCEmulationLog(7,"switched to execution mode");
}

void E5150::FDC::switchToResultMode (void)
{
	phase = FDC::PHASE::RESULT;
	makeDataRegisterReady();
	makeDataRegisterInReadMode();
	FDCEmulationLog(7,"switched to result mode");
}

void E5150::FDC::interruptPIC() const
{ picConnected.assertInterruptLine(PIC::IR_LINE::IR6,this); }

/*** Some utility functions ***/
bool E5150::FDC:: dataRegisterReady () const { return statusRegister & (1 << 7); }
static bool dataRegisterIsInReadMode (void) { return fdc->statusRegister & (1 << 6); }
static bool dataRegisterIsInWriteMode (void) { return !dataRegisterIsInReadMode(); }
static bool statusRegisterAllowReading (void) { return fdc->dataRegisterReady() && dataRegisterIsInReadMode() && fdc->statusRegisterRead; }
static bool statusRegisterAllowWriting (void) { return fdc->dataRegisterReady() && dataRegisterIsInWriteMode() && fdc->statusRegisterRead; }

static void setST0Flag (const FDC::ST0_FLAGS flag) { fdc->STRegisters[0] |= flag; }
static void resetST0Flag (const FDC::ST0_FLAGS flag) { fdc->STRegisters[0] &= ~flag; }

//TODO: implement interrupt on ready line change
void E5150::FDC::clock()
{
	++clockFromCommandStart;
	if (passClock == 0)
	{
		if (phase == PHASE::EXECUTION)
			selectedCommand->exec();
	}
	else
		--passClock;
}

//TODO: if selected while motor not on, does it unselect the previously selected floppy ?
//TODO: some motors should not run simultaneously
static void writeDOR(const uint8_t data)
{
	static Floppy100* previouslySelected = nullptr;

	fdc->floppyDrives[0].setMotorSpinning(data & (1 << 4));
	fdc->floppyDrives[1].setMotorSpinning(data & (1 << 5));
	fdc->floppyDrives[2].setMotorSpinning(data & (1 << 6));
	fdc->floppyDrives[3].setMotorSpinning(data & (1 << 7));

	if (fdc->floppyDrives[data & 0b11].select())
	{
		if (previouslySelected != nullptr)
			previouslySelected->unselect();
		previouslySelected = &fdc->floppyDrives[data & 0b11];
	}
}

static void writeDataRegister(const uint8_t data)
{
	static unsigned int writePos = 0;

	if (!selectedCommand)
	{
		if (E5150::FDC::instance->isBusy())
		{
			FDCEmulationLog(1,"New command issued while another command is being processed. Nothing done");
			return;
		}
		
		uint8_t commandIndex = data & 0b1111;

		//The first four digits of the first world identify the command except when the value equals 9 or 13
		//9 and 13 both identifies 2 commands, so by adding the fifth bit we can select one of both commands
		if ((commandIndex == 9) || (commandIndex == 13))
		{
			const bool hasCommandIndexOffset = data & 0b10000;

			if (hasCommandIndexOffset)
				commandIndex += (commandIndex == 9) ? 2 : 1;
		}
		
		selectedCommand = commands[commandIndex];
		
		FDCEmulationLog(5,"Selecting command '{}'",selectedCommand->m_name);
		selectedCommand->configurationBegin();
	}

	fdc->configurationDatas[writePos++] = data;
	fdc->statusRegisterRead = false;

	writePos %= selectedCommand->configurationWordsNumber;

	if (writePos == 0)
		selectedCommand->configurationEnd();
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
			if (statusRegisterRead)
			{
				if (dataRegisterIsInWriteMode())
				{
					writeDataRegister(data);
					return;
				}
				else
					FDCEmulationLog(1,"Data register {}", dataRegisterReady() ? "not in write mode" : "not ready");
			}
			else
				FDCEmulationLog(1,"Status register not read before writing to data register");
		}

		default:
			FDCEmulationLog(1,"Cannot write at address {:b}. Address should be 10b(0x3F2) for DOR or 100b (0x3F5) for data register",localAddress);
	}

	FDCEmulationLog(1,"Writing not done");
}

static void switchPhase (void)
{
	switch (fdc->phase)
	{
		case FDC::PHASE::COMMAND:
			fdc->switchToExecutionMode();
			break;

		case FDC::PHASE::EXECUTION:
			fdc->switchToResultMode();
			break;

		case FDC::PHASE::RESULT:
			fdc->switchToCommandMode();
			break;
	}
}

static uint8_t readDataRegister()
{
	static unsigned int resultWordPos=0;

	fdc->dataRegister = fdc->resultDatas[resultWordPos++];

	resultWordPos %= selectedCommand->resultWordsNumber;

	if (resultWordPos==0)
		switchPhase();
	
	fdc->statusRegisterRead = false;
	return fdc->dataRegister;
}

static uint8_t readStatusRegister()
{
	fdc->statusRegisterRead = true;
	return fdc->statusRegister;
}

uint8_t E5150::FDC::read	(const unsigned int localAddress)
{
	//No need to check addresse: from the doc the read signal only need to be a requested (end of page 4)
	if (readCommandInProcess)
	{
		makeDataRegisterNotReady();
		return readFromFloppy;
	}

	switch (localAddress)
	{
		case 2:
			return dorRegister;
		
		case 4:
			return readStatusRegister();
		
		case 5:
		{
			if (statusRegisterRead)
			{
				if (dataRegisterIsInReadMode())
					return readDataRegister();
				else
					FDCEmulationLog(1,"Data regisrer {}.",dataRegisterReady() ? "in write mode" : "not ready");
			}
			else
				FDCEmulationLog(1,"Status register not read before reading data register");
		} break;
		
		default:
			FDCEmulationLog(1,"Cannot read address {:b}. Address should be 10b (0x3F2) for DOR, 100b (0x3F4) or 101b (0x3F5) for status/data register",localAddress);
	}

	FDCEmulationLog(1,"Value outputed will be undetermined");
	return E5150::Util::undef;
}