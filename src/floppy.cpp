#include "floppy.hpp"

unsigned int Floppy100::floppyNumber = 0;

Floppy100::Floppy100(const std::string& path):driverNumber(floppyNumber++),m_timeToWait(0),m_lastTimeBeforeWait(Clock::now()),m_pcn(0)
{
	srand(time(NULL));

	if (!path.empty())
		open(path);
}

//TODO: will be removed
ID Floppy100::getID (void) const
{ return { m_pcn, 0 }; }

bool Floppy100::headLoaded() const
{ return !m_status.headUnloaded && ((Clock::now() - m_timing.lastTimeHeadLoadRequest >= m_timers.headLoad)); }

void Floppy100::loadHeads(void)
{ m_timing.lastTimeHeadLoadRequest = Clock::now(); }

bool Floppy100::select (void)
{
	if (!m_status.motorStoped)
	{
		m_status.selected = true;
		DEBUG("Floppy {}: selected", driverNumber);

		if (m_status.headUnloaded)
			loadHeads();
		
		return true;
	}
	else
		DEBUG("Floppy {}: can't be selected because motor is not spinning",driverNumber);
	
	return false;
}

void Floppy100::unselect (void)
{ m_status.selected = false; m_status.headUnloaded = true; DEBUG("Floppy {}: unselected", driverNumber); }

bool Floppy100::motorAtFullSpeed() const
{ return !m_status.motorStoped && ((Clock::now() - m_timing.lastTimeMotorStartRequest) >= m_timers.motorStart); }

void Floppy100::motorOn(void)
{
	if (m_status.motorStoped)
	{
		DEBUG("Floppy {}: motor start spinning", driverNumber);
		m_timing.lastTimeMotorStartRequest = Clock::now();
	}

	m_status.motorStoped = false;
}

void Floppy100::motorOff(void)
{
	if (!m_status.motorStoped)
		DEBUG("Floppy {}: motor stop spinning", driverNumber);
	m_status.motorStoped = true;
}

void Floppy100::setMotorSpinning(const bool spinning)
{ if (spinning) { motorOn(); } else { motorOff(); } }

bool Floppy100::waitingDone() const
{ return std::chrono::high_resolution_clock::now() - m_lastTimeBeforeWait >= m_timeToWait; }

void Floppy100::wait(const Milliseconds& toWait)
{
	if (!waitingDone())
		throw std::logic_error("Cannot wait will previous wait is not finished");

	m_lastTimeBeforeWait = Clock::now();
	m_timeToWait = toWait;
}

bool Floppy100::isReady() const
{ return m_status.selected && headLoaded() && motorAtFullSpeed(); }

void Floppy100::open(const std::string& path)
{
	m_file.close();

	if (path.empty())
		spdlog::warn("FLOPPY[{}]: empty path will leave the drive empty",driverNumber);
	else
	{
		m_file.open(path);

		if (!m_file.is_open())
			spdlog::error("FLOPPY[{}]: enable to open '{}'",driverNumber,path);
	}
}

//TODO: review this
void Floppy100::write (const uint8_t data, const size_t dataPos)
{
	if (m_file.is_open())
	{
		m_file.seekg(dataPos);
		m_file.put(data);
	}
}

bool Floppy100::stepHeadUp()
{
	if (m_pcn == m_geometry.cylinders - 1)
		return false;
	
	++m_pcn;
	return true;
}

bool Floppy100::stepHeadDown()
{
	if (m_pcn == 0)
		return false;
	
	--m_pcn;
	return true;
}

//TODO: what happen when the heads are unloaded
bool Floppy100::step(const bool direction, const Milliseconds& timeSinceLastStep, const bool firstStep)
{
	//const unsigned int cylinderCount = m_geometry.cylinders;
	//(ncn > cylinderCount) instead of (ncn >= cylinderCount) because 
	//40 cylinder from 0 to 39. ncn = 40 --> out of range
	//const unsigned int newCylinderNumber = (ncn > cylinderCount) ? cylinderCount : ncn;
	//const unsigned int offset = m_pcn > newCylinderNumber ?
	//	(m_pcn - newCylinderNumber) : (newCylinderNumber - m_pcn);
	
	//wait(m_timers.trackToTrack * offset);

	if (!firstStep && (timeSinceLastStep < m_timers.trackToTrack))
		return false;

	return direction ? stepHeadUp() : stepHeadDown();
}