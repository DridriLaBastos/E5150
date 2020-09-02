#include "floppy.hpp"

#define FLPDebug(REQUIRED_DEBUG_LEVEL,DEBUG_MSG,...) debug<REQUIRED_DEBUG_LEVEL>("FLOPPY {}: " DEBUG_MSG,driverNumber,##__VA_ARGS__)

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

bool E5150::Floppy100::headLoaded() const
{
	const bool headLoadFinish = (Clock::now() - timing.lastTimeHeadLoadRequest) >= timers.headLoad;

	if (status.headUnloaded)
		FLPDebug(4,"Head is not loaded");
	
	if (!headLoadFinish)
		FLPDebug(4,"Floppy {}: head loading isn't finish yet\n"
				"\tYou should wait {}ms after selecting the drive for the head to be loaded", driverNumber,timers.headLoad.count());
	return !status.headUnloaded && ((Clock::now() - timing.lastTimeHeadLoadRequest) >= timers.headLoad); }

void E5150::Floppy100::loadHeads(void)
{ timing.lastTimeHeadLoadRequest = Clock::now(); status.headUnloaded = false; }

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
{ status.selected = false; status.headUnloaded = true; FLPDebug(DEBUG_LEVEL_MAX,"Unselected"); }

static bool motorAtFullSpeed(const E5150::Floppy100* const flp)
{ return !flp->status.motorStoped && ((Clock::now() - flp->timing.lastTimeMotorStartRequest) >= flp->timers.motorStart); }

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

bool E5150::Floppy100::waitingDone() const
{ return std::chrono::high_resolution_clock::now() - lastTimeBeforeWait >= timeToWait; }

void E5150::Floppy100::wait(const Milliseconds& toWait)
{
	if (!waitingDone())
		throw std::logic_error("Cannot wait will previous wait is not finished");

	lastTimeBeforeWait = Clock::now();
	timeToWait = toWait;
}

bool E5150::Floppy100::isReady() const
{ return status.selected && headLoaded() && motorAtFullSpeed(this); }

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