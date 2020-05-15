#ifndef __RAM_HPP__
#define __RAM_HPP__

#include "util.hpp"

struct MapInfo
{
	unsigned int begin;
	unsigned int end;
	uint8_t(*readFunc)(unsigned int);
	void(*writeFunc)(unsigned int, uint8_t);
};

class RAM
{
	public:
		RAM(void);
		~RAM(void);

	public:
		uint8_t read(const unsigned int address);

		void write(const unsigned int address, const uint8_t data);
		void load(const std::string path, size_t pos);
		void map(const MapInfo info);

		friend class CPU;

	private:
		uint8_t* const m_ram;
		std::vector<MapInfo> m_mappedDevices;
};

#endif