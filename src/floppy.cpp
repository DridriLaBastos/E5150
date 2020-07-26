#include "floppy.hpp"

unsigned int Floppy100::floppyNumber = 0;

Floppy100::Floppy100(const std::string& path):driverNumber(floppyNumber++),m_readPos(0),m_timeToWait(sf::Time::Zero)
{
	srand(time(NULL));

	if (!path.empty())
		open(path);
	
	m_id.track = 0xA;   m_id.sector = 2;
}

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
		m_clock.restart();
	}
}

//TODO: take in account that the floppy is spinning
ID Floppy100::getID() const { return m_id; }

//TODO: what happen when the heads are unloaded
template<>
std::pair<bool,sf::Time> Floppy100::command<Floppy100::COMMAND::SEEK>(const unsigned int newTrack)
{
	std::pair<bool, sf::Time> ret (false,sf::Time::Zero);
		//? std::pair<bool,sf::Time>{false,sf::Time::Zero} : std::pair<bool,sf::Time>{true,sf::Time::Zero};

	if (ret.first)
	{
		const int deltaPos = (int)m_id.track - (int)newTrack;
		m_id.track = newTrack;
		const sf::Time timeToWait = sf::milliseconds(((deltaPos < 0) ? -deltaPos : deltaPos)*8);//Track to track movement takes 8 ms
		ret.second = timeToWait;
		m_timeToWait = timeToWait;
	}
	m_clock.restart();
	return ret; 
}

template<>
std::pair<bool,sf::Time> Floppy100::command<Floppy100::COMMAND::READ>(void)
{ return  std::pair<bool,sf::Time>(false,sf::seconds(1)); }