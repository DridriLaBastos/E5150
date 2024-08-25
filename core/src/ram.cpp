#include "core/ram.hpp"
#include "core/arch.hpp"
#include "core/util.hpp"

E5150::RAM::RAM(): m_ram(new uint8_t[0x100000]) {}

static void UpdateBus(const unsigned int address, const uint8_t data)
{
	E5150::Arch::addressBus = address;
	E5150::Arch::dataBus = data;
}

uint8_t E5150::RAM::Read(const unsigned int address) const
{
	const uint8_t data = m_ram.get()[address];
	UpdateBus(data,address);
	return data;
}
void E5150::RAM::Write(const unsigned int address, const uint8_t data)
{
	m_ram.get()[address] = data;
	UpdateBus(address,data);
}

void E5150::RAM::LoadFromFile(const std::filesystem::path path, size_t startPos)
{
	std::ifstream stream(path);

	if (!stream.is_open())
	{
		E5150_WARNING("Cannot open file '{}'",path.c_str());
		return;
	}

	while (!stream.eof())
	{
		m_ram.get()[startPos++] = stream.get();
	}
}

#if 0
void RAM::load (const std::string path, size_t pos)
{
	std::ifstream stream (path,std::ios_base::binary);

	if (!stream.is_open())
	{
		E5150_WARNING("cannot open file '{}'",path);
	}
	else
	{
		//TODO: Check if the file is larger than the memory available
		while (!stream.eof() && pos < 0x100000)
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

uint8_t *RAM::GetRamData(const unsigned int offset) const noexcept {
	return &m_ram[offset];
}
#endif
