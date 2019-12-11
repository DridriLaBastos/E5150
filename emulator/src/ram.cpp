#include "ram.hpp"

RAM::RAM (void): m_ram (new uint8_t [0xF00000]), m_mappedDevices() {m_mappedDevices.reserve(5);}
RAM::~RAM() { delete[]m_ram; }

/* TODO: Algorithme par dichotomie pour la recherche */
uint8_t RAM::read (const unsigned int address)
{
	auto f = std::find_if(m_mappedDevices.begin(), m_mappedDevices.end(), 
	[address] (MapInfo info) -> bool {return (address >= info.begin) && (address <= info.end);});

	const uint8_t tmp = (f == m_mappedDevices.end()) ? m_ram[address] : f->readFunc(address - f->begin);

#if defined SEE_RAM_RW || SEE_RAM_READ || SEE_ALL
	std::cout << std::hex << "0x" << (int)tmp << " --> " << "0x" << address << std::dec << std::endl;
#endif

	return tmp;
}

void RAM::write (const unsigned int address, const uint8_t data)
{
	auto f = std::find_if(m_mappedDevices.begin(), m_mappedDevices.end(), 
	[this, address] (MapInfo info) -> bool {return (address >= info.begin) && (address <= info.end);});

	if (f == m_mappedDevices.end())
			m_ram[address] = data;
	else
		f->writeFunc(address - f->begin, data);
#if defined SEE_RAM_RW || SEE_RAM_WRITE || SEE_ALL
	std::cout << std::hex << "0x" << address << " <-- " << "0x" << (int)data << std::dec << std::endl;
#endif
}

void RAM::load (const std::string path, size_t pos)
{
	std::ifstream stream (path);

	if (!stream.is_open())
	{
		std::cerr << "ERROR when openning " << path << std::endl;
		std::cerr << "      file not loaded" << std::endl;
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