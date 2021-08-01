#include "ram.hpp"
#include "arch.hpp"

RAM::RAM(): mRamPtr(new uint8_t[0x100000]), m_mappedDevices()
{
	m_mappedDevices.reserve(5);
	m_ram = mRamPtr.get();
}

uint8_t RAM::read(const unsigned int address) const
{
	const uint8_t data = m_ram[address];
#if defined(SEE_RAM_RW) || defined(SEE_RAM_READ) || defined(SEE_ALL)
	if (E5150::Util::CURRENT_DEBUG_LEVEL == DEBUG_LEVEL_MAX)
		std::cout << std::hex << data << " --> " << address << std::dec << std::endl;
#endif
	return data;
}
void RAM::write(const unsigned int address, const uint8_t data)
{
	m_ram[address] = data;
#if defined(SEE_RAM_RW) || defined(SEE_RAM_WRITE) || defined(SEE_ALL)
	if (E5150::Util::CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_MAX)
		std::cout << std::hex << "0x" << address << " <-- " << "0x" << data << std::dec << std::endl;
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
