#include "floppy.hpp"

unsigned int Floppy::floppyNumber = 0;

Floppy::Floppy(const std::string& path):driverNumber(floppyNumber++),m_readPos(0)
{
	srand(time(NULL));

	if (!path.empty())
		open(path);
}

void Floppy::open(const std::string& path)
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

uint8_t Floppy::read(const size_t dataPos)
{
	uint8_t ret = rand();

	if (m_file.is_open())
	{
		m_file.seekg(dataPos);
		ret = m_file.get();
	}

	return ret;
}

unsigned int Floppy::moveHeadToTrack(const unsigned int newTrack)
{ return 8; }

void Floppy::write (const uint8_t data, const size_t dataPos)
{
	if (m_file.is_open())
	{
		m_file.seekg(dataPos);
		m_file.put(data);
	}
}