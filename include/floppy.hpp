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

		bool isReady (void) const;

		void write (const uint8_t data, const size_t dataPos);
		ID getID (void) const;

		template <class CommandName, class... Args>
		std::pair<bool, sf::Time> performeCommand (Args... args)
		{
			return (m_clock.getElapsedTime() >= m_timeToWait ) ? command<CommandName>(args...) : std::pair <bool, sf::Time>{false, sf::Time::Zero};
		}

	public:
		const unsigned int driverNumber;
		
	public:
		struct COMMAND
		{
			class SEEK;
			class READ;
		};

	private:
		template <class CommandName, class... Args>
		std::pair<bool, sf::Time> command(Args... args);

	private:
		std::fstream m_file;
		size_t m_readPos;
		bool m_isOpen;
		bool m_headLoaded;
		bool m_floppyReady;
		sf::Clock m_clock;
		sf::Time m_timeToWait;
		static unsigned int floppyNumber;
		unsigned int m_totalTrackNumber;
		ID m_id;
};

#endif