#ifndef __RAM_HPP__
#define __RAM_HPP__

#include "bus.hpp"
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

	public:
		uint8_t read(const unsigned int address) const;
		void write(const unsigned int address, const uint8_t data);
	
		void load(const std::string path, size_t pos);
		void map(const MapInfo info);
		uint8_t* GetRamData(const unsigned int offset = 0) const noexcept;
		friend class CPU;

	private:
		std::unique_ptr<uint8_t> mRamPtr;
		uint8_t* m_ram;
		std::vector<MapInfo> m_mappedDevices;

};

#endif
