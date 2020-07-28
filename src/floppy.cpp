#include "floppy.hpp"

unsigned int Floppy100::floppyNumber = 0;

Floppy100::Floppy100(const std::string& path):driverNumber(floppyNumber++),m_readPos(0),m_timeToWait(0)
{
	srand(time(NULL));

	if (!path.empty())
		open(path);
	
	m_id.track = 0xA;   m_id.sector = 2;
}

bool Floppy100::select(void)
{
	if (m_spinning)
	{
		m_selected = true;
		DEBUG("Floppy {}: selected", driverNumber);
	}
	else
		DEBUG("Floppy {}: can't be selected because motor is not spinning",driverNumber);
	
	return m_spinning;
}
void Floppy100::unselect (void)
{ m_selected = false; DEBUG("Floppy {}: unselected",driverNumber); }

bool Floppy100::waitingDone() const
{ return std::chrono::high_resolution_clock::now() - m_lastTimeBeforeWait >= m_timeToWait; }

//TODO: check needed ?
void Floppy100::waitMilliseconds (const unsigned int millisecondsToWait)
{
	if (!waitingDone())
		throw std::logic_error("Cannot wait will previous wait is not finished");
	
	m_lastTimeBeforeWait = std::chrono::high_resolution_clock::now();
	m_timeToWait = std::chrono::milliseconds(millisecondsToWait);
}

//TODO: timing
void Floppy100::setMotorSpinning (const bool spinning)
{
	if (spinning != m_spinning)
		DEBUG("Floppy {}: motor {} spinning",driverNumber,spinning ? "start" : "stop");
	
	if (spinning)
		waitMilliseconds(500);

	m_spinning = spinning;
}

bool Floppy100::isReady() const
{ return m_selected && m_spinning && waitingDone(); }

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

/*uint8_t Floppy100::read(const size_t dataPos)
{
	uint8_t ret = rand();

	if (m_file.is_open())
	{
		m_file.seekg(dataPos);
		ret = m_file.get();
	}

	return ret;
}*/

//TODO: review this
void Floppy100::write (const uint8_t data, const size_t dataPos)
{
	if (m_file.is_open())
	{
		m_file.seekg(dataPos);
		m_file.put(data);
	}
}

//TODO: take in account that the floppy is spinning
ID Floppy100::getID() const { return m_id; }

//TODO: what happen when the heads are unloaded
template<>
std::pair<bool, uint8_t> Floppy100::command<Floppy100::COMMAND::STEP>(const bool direction)
{
	std::pair<bool, uint8_t> ret (false,0);

	return ret; 
}