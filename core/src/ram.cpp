#include "ram.hpp"
#include "arch.hpp"

RAM::RAM(): mRamPtr(new uint8_t[0x100000]), m_mappedDevices()
{
	m_mappedDevices.reserve(5);
	m_ram = mRamPtr.get();
}

/* TODO: Algorithme par dichotomie pour la recherche */
uint8_t RAM::read (const unsigned int address)
{
	auto f = std::find_if(m_mappedDevices.begin(), m_mappedDevices.end(), 
	[address] (MapInfo info) -> bool {return (address >= info.begin) && (address <= info.end);});

	const uint8_t tmp = (f == m_mappedDevices.end()) ? m_ram[address] : f->readFunc(address - f->begin);

#if defined(SEE_RAM_RW) || defined(SEE_RAM_READ) || defined(SEE_ALL)
	if (E5150::Util::CURRENT_DEBUG_LEVEL == DEBUG_LEVEL_MAX)
		std::cout << std::hex << "0x" << (int)tmp << " --> " << "0x" << address << std::dec << std::endl;
#endif

	return tmp;
}

void RAM::write (const unsigned int address, const uint8_t data)
{
	auto f = std::find_if(m_mappedDevices.begin(), m_mappedDevices.end(), 
	[address] (MapInfo info) -> bool {return (address >= info.begin) && (address <= info.end);});

	if (f == m_mappedDevices.end())
			m_ram[address] = data;
	else
		f->writeFunc(address - f->begin, data);
#if defined(SEE_RAM_RW) || defined(SEE_RAM_WRITE) || defined(SEE_ALL)
	if (E5150::Util::CURRENT_DEBUG_LEVEL == DEBUG_LEVEL_MAX)
		std::cout << std::hex << "0x" << address << " <-- " << "0x" << (int)data << std::dec << std::endl;
#endif
}

void RAM::read()
{
	dataBus = m_ram[addressBus];
#if defined(SEE_RAM_RW) || defined(SEE_RAM_READ) || defined(SEE_ALL)
	// if (E5150::Util::CURRENT_DEBUG_LEVEL == DEBUG_LEVEL_MAX)
		// std::cout << std::hex << dataBus << " --> " << addressBus << std::dec << std::endl;
#endif
}
void RAM::write()
{
	m_ram[addressBus] = dataBus;
#if defined(SEE_RAM_RW) || defined(SEE_RAM_WRITE) || defined(SEE_ALL)
	// if (E5150::Util::CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_MAX)
		// std::cout << std::hex << "0x" << addressBus << " <-- " << "0x" << dataBus << std::dec << std::endl;
#endif
}

void RAM::load (const std::string path, size_t pos)
{
	std::ifstream stream (path);

	if (!stream.is_open())
	{
		WARNING("cannot open file '{}'",path);
	}
	else
	{
		while (!stream.eof())
		{
			const uint8_t tmp = stream.get();
			m_ram[pos++] = tmp;
		}
	}
}

void RAM::map  (const MapInfo info)
{
	m_mappedDevices.emplace(std::find_if(
		m_mappedDevices.begin(), m_mappedDevices.end(), [&info](MapInfo i){return i.begin > info.begin;}), info);
}