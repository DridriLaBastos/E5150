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
		 * \return (operation possible, time in milliseconds to perform the whole action) 
		 */
		std::pair<bool, unsigned int> moveHeadToTrack (const unsigned int newTrack);

		ID getID (void) const;

		bool areHeadsLoaded(void) const
		{ return m_clock.getElapsedTime().asMilliseconds() >= 35; }

		bool isMotorAtFullSpeed (void) const
		{ return m_clock.getElapsedTime().asMilliseconds() >= 500; }
		
		/**
		 * @brief Prepare the floppy drive to performe an action.
		 * 
		 * This function restart the clock so that we can measure time and tell later if an action can be done or not.
		 * For exemple, if we want to start the motor we should first call prepare. This will restart the clock. Then,
		 * to know if the mototor is at full speed or not, we should call the funtion isMotorAtFullSpeed. This function will
		 * return true if the time of the clock is greater than the time needed for the motor to be at full speed or not.
		 */
		void pepare (void) { m_clock.restart(); }
		void write (const uint8_t data, const size_t dataPos);

		const unsigned int driverNumber;

		struct COMMAND
		{
			class SEEK;
		};

		template <class CommandName, class... Args>
		std::pair<bool, sf::Time> performeCommand (Args... args)
		{ return (m_clock.getElapsedTime() >= m_timeToWait ) ? command<CommandName>(args...) : std::pair <bool, sf::Time>{false, sf::Time::Zero}; }

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