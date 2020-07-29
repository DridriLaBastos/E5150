#ifndef FLOPPY_HPP
#define FLOPPY_HPP

#include "util.hpp"

struct ID
{
	unsigned int cylinder;
	unsigned int sector;
};

struct Geometry
{
	const unsigned int cylinders;
	const unsigned int heads;
	const unsigned int sectors;//Sector/cylinders
	const bool doubleSided;
};

struct Status
{
	bool headUnloaded;
	bool motorStoped;
	bool selected;
};

struct Timing
{
	std::chrono::time_point<Clock> lastTimeHeadLoadRequest;
	std::chrono::time_point<Clock> lastTimeMotorStartRequest;
};

struct Timer
{
	Milliseconds motorStart;
	Milliseconds headLoad;
	Milliseconds headUnload;
	Milliseconds trackToTrack;
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

		void write (const uint8_t data, const size_t dataPos);
		ID getID (void) const;

		bool isReady (void) const;
		void setMotorSpinning (const bool spinning);
		bool step(const bool direction, const Milliseconds& timeSinceLastStep, const bool firstStep);
		void motorOn  (void);
		void motorOff (void);
		bool select   (void);
		void unselect (void);

	public:
		const unsigned int driverNumber;
		unsigned int m_pcn = 0;

	private:
		void loadHeads (void);
		bool waitingDone (void) const;
		void wait (const Milliseconds& toWait);

		bool headLoaded (void) const;
		bool motorAtFullSpeed (void) const;

		bool stepHeadUp (void);
		bool stepHeadDown (void);

	private:
		std::fstream m_file;
		bool m_isOpen;

		std::chrono::time_point<Clock> m_lastTimeBeforeWait;
		Milliseconds m_timeToWait;

		static unsigned int floppyNumber;

		Geometry m_geometry { 40, 1, 8, false };
		Status m_status { true, true, false };
		Timer m_timers { Milliseconds(500), Milliseconds(35), Milliseconds(240), Milliseconds(8) };
		Timing m_timing;
};

#endif