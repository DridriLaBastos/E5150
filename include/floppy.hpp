#ifndef FLOPPY_HPP
#define FLOPPY_HPP

#include "util.hpp"

struct ID
{
	unsigned int track;
	unsigned int sector;
};

//TODO: implements error state
/**
 * Floppy model 100 implementation
 */
class Floppy100
{
	public:
		Floppy100(const std::string& path = "");
		void open (const std::string& path);
		uint8_t read(const size_t dataPos);
		/**
		 * Moves the head to a new track
		 * 
		 * \param newTrack the new track
		 * 
		 * \return time in ms to perform the action 
		 */
		unsigned int moveHeadToTrack (const unsigned int newTrack);

		ID getID (void) const;

		bool areHeadsLoaded(void) const
		{ return m_clock.getElapsedTime().asMilliseconds() < 240; }

		unsigned int loadHeads(void) { m_clock.restart(); return 35; }
		void write (const uint8_t data, const size_t dataPos);

		const unsigned int driverNumber;

	private:
		std::fstream m_file;
		size_t m_readPos;
		bool m_isOpen;
		bool m_headLoaded;
		sf::Clock m_clock;
		static unsigned int floppyNumber;
		ID m_id;
};

#endif