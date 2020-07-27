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
		bool select(void);
		void unselect (void);
		void setMotorSpinning (const bool spinning);

		void write (const uint8_t data, const size_t dataPos);
		ID getID (void) const;

		template <class CommandName, class... Args>
		std::pair<bool, uint8_t> performeCommand (Args... args)
		{ return isReady() ? command<CommandName>(args...) : std::pair <bool, uint8_t>{false, 0}; }

	public:
		const unsigned int driverNumber;
		
	public:
		struct COMMAND
		{
			class STEP;
		};

	private:
		template <class CommandName, class... Args>
		std::pair<bool, uint8_t> command(Args... args);

		bool waitingDone (void) const;
		void waitMilliseconds (const unsigned int millisecondsToWait);

	private:
		std::fstream m_file;
		size_t m_readPos;
		bool m_isOpen;
		bool m_headLoaded;
		bool m_floppyReady;

		bool m_selected = false;
		bool m_spinning = false;

		std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTimeBeforeWait;
		std::chrono::milliseconds m_timeToWait;

		static unsigned int floppyNumber;
		unsigned int m_totalTrackNumber;
		ID m_id;
};

#endif