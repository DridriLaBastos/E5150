#ifndef FLOPPY_HPP
#define FLOPPY_HPP

#include "util.hpp"

struct ID
{
	unsigned int cylinder; //C: the current selected cylinder
	unsigned int headAddress;//H: head number
	unsigned int record;//R: sector number which will be read/write
	unsigned int number;//N: number of data byte written in a sector

	ID (unsigned int c=0,unsigned int h=0, unsigned int r=1,unsigned int n=0): cylinder(c), headAddress(h), record(r), number(n) { ASSERT(r >= 1); }
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
	bool headUnloaded = false;
	bool motorStoped = true;
	bool selected = false;
	bool writeProtected = false;
	bool RWOperationHappened=false;
	unsigned int headAddress = 0;
};

struct Timing
{
	std::chrono::time_point<Clock> lastTimeHeadLoadRequest;
	std::chrono::time_point<Clock> lastTimeMotorStartRequest;
	std::chrono::time_point<Clock> endOfLastRWOperation;
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

		uint8_t read (void);
		void write (const uint8_t data, const size_t dataPos);

		//TODO: better getID function
		const ID getID (void) const;

		bool headLoaded (void) const;
		bool isReady (void) const;
		void loadHeads(void);
		void setMotorSpinning (const bool spinning);
		bool step(const bool direction, const Milliseconds& timeSinceLastStep, const bool firstStep);
		void motorOn  (void);
		void motorOff (void);
		bool select   (void);
		void unselect (void);
		void setHeadAddress (const unsigned int headAddress);

		uint8_t getStatusRegister3 (void) const;

		const unsigned int driverNumber;
		ID currentID;

		std::fstream file;
		bool isOpen;

		std::chrono::time_point<Clock> lastTimeBeforeWait;
		Milliseconds timeToWait;

		static unsigned int floppyNumber;

		Geometry geometry { 40, 1, 8, false };
		Status status;
		Timer timers { Milliseconds(500), Milliseconds(35), Milliseconds(240), Milliseconds(8) };
		Timing timing;

		private:
			bool inserted = false;
	};
}

#endif