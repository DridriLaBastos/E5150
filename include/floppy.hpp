#ifndef FLOPPY_HPP
#define FLOPPY_HPP

#include "util.hpp"

class Floppy
{
	public:
		Floppy(const std::string& path = "");
		void open (const std::string& path);
		uint8_t read(const size_t dataPos);
		void write (const uint8_t data, const size_t dataPos);

		const unsigned int driverNumber;
	private:
		std::fstream m_file;
		size_t m_readPos;
		bool m_isOpen;
		static unsigned int floppyNumber;
};

#endif