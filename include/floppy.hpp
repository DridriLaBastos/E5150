#ifndef FLOPPY_HPP
#define FLOPPY_HPP

#include "util.hpp"

struct ID
{
	unsigned int cylinder; //C: the current selected cylinder
	unsigned int headAddress;//H: head number 
	unsigned int record;//R: sector number which will be read/write
	unsigned int number;//N: number of data byte written in a sector
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
	unsigned int headAddress;
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
namespace E5150
{
	struct Floppy100
	{
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
		void setHeadAddress (const unsigned int headAddress) { status.headAddress=headAddress; }
		void resetHeadAddress (void) { setHeadAddress(0); }

		const unsigned int driverNumber;
		unsigned int pcn = 0;

		std::fstream file;
		bool isOpen;

		std::chrono::time_point<Clock> lastTimeBeforeWait;
		Milliseconds timeToWait;

		static unsigned int floppyNumber;

		Geometry geometry { 40, 1, 8, false };
		Status status { true, true, false, 0 };
		Timer timers { Milliseconds(500), Milliseconds(35), Milliseconds(240), Milliseconds(8) };
		Timing timing;
	};
}

#endif