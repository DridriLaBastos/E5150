#include "floppy.hpp"

#define FLPDebug(REQUIRED_DEBUG_LEVEL,DEBUG_MSG,...) debug<REQUIRED_DEBUG_LEVEL>("FLOPPY {}: " DEBUG_MSG,driverNumber,##__VA_ARGS__)
#define EXTERNAL_FLPDebug(REQUIRED_DEBUG_LEVEL,DEBUG_MSG,...) debug<REQUIRED_DEBUG_LEVEL>("FLOPPY {}: " DEBUG_MSG,flp->driverNumber,##__VA_ARGS__)

unsigned int E5150::Floppy100::floppyNumber = 0;

E5150::Floppy100::Floppy100(const std::string& path):driverNumber(floppyNumber++),timeToWait(0),lastTimeBeforeWait(Clock::now()),pcn(0)
{
	srand(time(NULL));

	if (!path.empty())
		open(path);
}

//TODO: will be removed
ID E5150::Floppy100::getID (void) const
{ return { pcn, 0,0,0 }; }

void E5150::Floppy100::setHeadAddress(const unsigned int headAddress)
{ status.headAddress=headAddress; }

static void loadHeads(E5150::Floppy100* const flp)
{ flp->timing.lastTimeHeadLoadRequest = Clock::now(); flp->status.headUnloaded = false; }

bool E5150::Floppy100::select (void)
{
	if (!status.motorStoped)
	{
		status.selected = true;
		FLPDebug(DEBUG_LEVEL_MAX,"Selected");

		if (status.headUnloaded)
			loadHeads(this);
		
		return true;
	}
	else
		FLPDebug(8,"Not selected because motor is not spinning");
	
	return false;
}

void E5150::Floppy100::unselect (void)
{ status.selected = false; status.headUnloaded = true; FLPDebug(DEBUG_LEVEL_MAX,"Unselected"); }

void E5150::Floppy100::motorOn(void)
{
	if (status.motorStoped)
	{
		FLPDebug(10,"Motor start spinning");
		timing.lastTimeMotorStartRequest = Clock::now();
	}

	status.motorStoped = false;
}

void E5150::Floppy100::motorOff(void)
{
	if (!status.motorStoped)
		FLPDebug(DEBUG_LEVEL_MAX,"Motor stop spinning");
	status.motorStoped = true;
}

void E5150::Floppy100::setMotorSpinning(const bool spinning)
{ if (spinning) { motorOn(); } else { motorOff(); } }

//TODO: handle desynchronization
static bool waitingDone(const E5150::Floppy100* const flp)
{ return std::chrono::high_resolution_clock::now() - flp->lastTimeBeforeWait >= flp->timeToWait; }

static void wait(E5150::Floppy100* const flp, Milliseconds& toWait)
{
	if (!waitingDone(flp))
		throw std::logic_error("Cannot wait will previous wait is not finished");

	flp->lastTimeBeforeWait = Clock::now();
	flp->timeToWait = toWait;
}

static bool headLoaded(const E5150::Floppy100* const flp)
{
	const bool headLoadFinish = (Clock::now() - flp->timing.lastTimeHeadLoadRequest) >= flp->timers.headLoad;

	if (flp->status.headUnloaded)
		EXTERNAL_FLPDebug(4,"Head is not loaded");
	
	if (!headLoadFinish)
		EXTERNAL_FLPDebug(4,"Head loading isn't finish yet\n"
				"\tYou should wait {}ms after selecting the drive for the head to be loaded",flp->timers.headLoad.count());
	return !flp->status.headUnloaded && ((Clock::now() - flp->timing.lastTimeHeadLoadRequest) >= flp->timers.headLoad);
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
{ return status.selected && headLoaded(this) && motorAtFullSpeed(this) && correctHeadAddress(this); }

void E5150::Floppy100::open(const std::string& path)
{
	file.close();

	if (path.empty())
		WARNING("FLOPPY[{}]: empty path will leave the drive empty",driverNumber);
	else
	{
		file.open(path);

		if (!file.is_open())
			WARNING("FLOPPY[{}]: enable to open '{}'",driverNumber,path);
	}
}

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
	if (flp->pcn == flp->geometry.cylinders - 1)
		return false;
	
	++flp->pcn;
	return true;
}

static bool stepHeadDown(E5150::Floppy100* const flp)
{
	if (flp->pcn == 0)
		return false;
	
	--flp->pcn;
	return true;
}

//TODO: what happen when the heads are unloaded
bool E5150::Floppy100::step(const bool direction, const Milliseconds& timeSinceLastStep, const bool firstStep)
{
	if (!status.selected)
		FLPDebug(5,"Step while not selected");

	if (!firstStep && (timeSinceLastStep < timers.trackToTrack))
	{
		FLPDebug(4,"timestep of {} ms, should be {} ms", timeSinceLastStep.count(), timers.trackToTrack.count());
		return false;
	}

	return direction ? stepHeadUp(this) : stepHeadDown(this);
}

uint8_t E5150::Floppy100::getStatusRegister3() const
{
	const unsigned int FT = 0;
	const unsigned int WP = status.writeProtected ? 1 << 6 : 0;
	const unsigned int RDY = isReady() ? 1 << 5: 0;
	const unsigned int T0 = pcn == 0 ? 1 << 4: 0;
	//const unsigned int TS = 0; single sided but here for completness
	const unsigned int HD = status.headAddress << 2;
	const unsigned int USx = driverNumber;

	return FT | WP | RDY | T0 | HD | USx;
}