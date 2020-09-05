#include "floppy.hpp"

#define FLPDebug(REQUIRED_DEBUG_LEVEL,DEBUG_MSG,...) debug<REQUIRED_DEBUG_LEVEL>("FLOPPY {}: " DEBUG_MSG,driverNumber,##__VA_ARGS__)
#define EXTERNAL_FLPDebug(REQUIRED_DEBUG_LEVEL,DEBUG_MSG,...) debug<REQUIRED_DEBUG_LEVEL>("FLOPPY {}: " DEBUG_MSG,flp->driverNumber,##__VA_ARGS__)

unsigned int E5150::Floppy100::floppyNumber = 0;

E5150::Floppy100::Floppy100(const std::string& path):driverNumber(floppyNumber++),timeToWait(0),lastTimeBeforeWait(Clock::now()),currentID({1,0,0,0})
{
	srand(time(NULL));

	if (!path.empty())
		open(path);
}

void E5150::Floppy100::open(const std::string& path)
{
	file.close();
	inserted = false;

	if (path.empty())
	{
		WARNING("FLOPPY[{}]: empty path will leave the drive empty",driverNumber);
		return;
	}

	file.open(path);
	inserted = file.is_open();

	if (!file.is_open())
	{
		WARNING("FLOPPY[{}]: enable to open '{}'",driverNumber,path);
		return;
	}
}

//TODO: will be removed
const ID E5150::Floppy100::getID (void) const
{ return { inserted ? currentID.cylinder :0,0,0, inserted ? 512:0 }; }

void E5150::Floppy100::setHeadAddress(const unsigned int headAddress)
{ status.headAddress=headAddress; }

void E5150::Floppy100::loadHeads ()
{
	if (!status.selected)
		return;

	timing.lastTimeHeadLoadRequest = Clock::now();
	status.headUnloaded = false;
}

bool E5150::Floppy100::select (void)
{
	if (!status.motorStoped)
	{
		status.selected = true;
		FLPDebug(DEBUG_LEVEL_MAX,"Selected");

		if (status.headUnloaded)
			loadHeads();
		
		return true;
	}
	else
		FLPDebug(8,"Not selected because motor is not spinning");
	
	return false;
}

void E5150::Floppy100::unselect (void)
{
	status.selected = false;
	status.headUnloaded = true;

	FLPDebug(DEBUG_LEVEL_MAX,"Unselected");
}

void E5150::Floppy100::motorOn(void)
{
	if (!inserted)
		return;

	if (status.motorStoped)
	{
		FLPDebug(10,"Motor start spinning");
		timing.lastTimeMotorStartRequest = Clock::now();
	}

	status.motorStoped = false;
}

void E5150::Floppy100::motorOff(void)
{
	if (!inserted)
		return;
	
	if (!status.motorStoped)
		FLPDebug(DEBUG_LEVEL_MAX,"Motor stop spinning");
	status.motorStoped = true;
}

void E5150::Floppy100::setMotorSpinning(const bool spinning)
{
	if (!inserted)
		return;
	
	if (spinning) { motorOn(); } else { motorOff(); }
}

bool E5150::Floppy100::headLoaded() const
{
	if (!inserted)
		return false;

	if (status.headUnloaded)
	{
		FLPDebug(4,"Head is not loaded");
		return false;
	}

	//If a R/W operation happened and ended, cheking that the head is still loaded
	if (status.RWOperationHappened)
		return Clock::now() - timing.endOfLastRWOperation < timers.headUnload;
	
	//Checking if the loading part is finish
	const bool headLoadFinish = (Clock::now() - timing.lastTimeHeadLoadRequest) >= timers.headLoad;

	if (!headLoadFinish)
		FLPDebug(4,"Head loading isn't finish yet\n"
				"\tYou should wait {}ms after selecting the drive for the head to be loaded",timers.headLoad.count());

	return headLoadFinish;
}

static bool motorAtFullSpeed(const E5150::Floppy100* const flp)
{ return !flp->status.motorStoped && ((Clock::now() - flp->timing.lastTimeMotorStartRequest) >= flp->timers.motorStart); }

static bool correctHeadAddress (const E5150::Floppy100* const flp)
{
	if (flp->status.headAddress)
		EXTERNAL_FLPDebug(DEBUG_LEVEL_MAX,"Single sided. Head address can only be 0 but found {}",flp->status.headAddress);
	
	return !flp->status.headAddress;
}

bool E5150::Floppy100::isReady() const
{ return status.selected && headLoaded() && motorAtFullSpeed(this) && correctHeadAddress(this); }

//TODO: review this
void E5150::Floppy100::write (const uint8_t data, const size_t dataPos)
{
	if (file.is_open())
	{
		file.seekg(dataPos);
		file.put(data);
	}
}

static bool stepHeadUp(E5150::Floppy100* const flp)
{
	if (flp->currentID.cylinder == flp->geometry.cylinders - 1)
		return false;
	
	++flp->currentID.cylinder;
	return true;
}

static bool stepHeadDown(E5150::Floppy100* const flp)
{
	if (flp->currentID.cylinder == 0)
		return false;
	
	--flp->currentID.cylinder;
	return true;
}

//TODO: what happen when the heads are unloaded
//TODO: check cylinder addressing
bool E5150::Floppy100::step(const bool direction, const Milliseconds& timeSinceLastStep, const bool firstStep)
{
	if (!inserted)
		return true;

	if (!status.selected)
		FLPDebug(5,"Step while not selected");

	if (!firstStep && (timeSinceLastStep < timers.trackToTrack))
	{
		FLPDebug(4,"timestep of {} ms, should be {} ms", timeSinceLastStep.count(), timers.trackToTrack.count());
		return false;
	}

	const unsigned int newFilePos = currentID.cylinder*8 + (currentID.record-1);//got to the beginning of the sector
	file.seekp(newFilePos);
	return direction ? stepHeadUp(this) : stepHeadDown(this);
}

uint8_t E5150::Floppy100::getStatusRegister3() const
{
	const unsigned int FT = 0;
	const unsigned int WP = status.writeProtected ? 1 << 6 : 0;
	const unsigned int RDY = isReady() ? 1 << 5: 0;
	const unsigned int T0 = currentID.cylinder == 0 ? 1 << 4: 0;
	//const unsigned int TS = 0; single sided but here for completness
	const unsigned int HD = status.headAddress << 2;
	const unsigned int USx = driverNumber;

	return FT | WP | RDY | T0 | HD | USx;
}

uint8_t E5150::Floppy100::read()
{
	uint8_t ret;

	//If non inserted the returned value will be indetermined
	if (inserted)
	{
		ret = file.get();
		++currentID.number;

		if (currentID.number == 512)
		{
			currentID.number = 0;
			++currentID.record;
		}

		if (currentID.record == 9) {}
			//TODO: end of track
	}

	return ret;
}