#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "fdc.hpp"

namespace E5150
{
	namespace FDC_COMMAND
	{
		struct Command
		{
			public:
				Command (const std::string& name = "",const unsigned int configNumber=9,const unsigned int resultNumber=7);

				virtual void exec (void);
				const std::string m_name;

				const unsigned int configurationWordsNumber;
				const unsigned int resultWordsNumber;
				unsigned int m_clockWait;

				virtual void configurationBegin  (void);
				virtual void configurationEnd (void);

				Floppy100* floppyToApply = nullptr;
			
		};

		class ReadData: public Command
		{
			virtual void exec (void) final;
			virtual void configurationEnd(void) final;
			bool loadHeadRequested=false;
		};

		class WriteData: public Command {};

		class ReadDeletedData: public Command {};

		class WriteDeletedData: public Command {};

		class ReadATrack: public Command {};

		class ScanLEQ: public Command {};

		class ReadID: public Command
		{
			virtual void exec (void) final;
			public:
				ReadID(void);
		};

		class ScanHEQ: public Command {};

		class FormatTrack: public Command {};

		class ScanEqual: public Command {};

		class Recalibrate: public Command
		{
			virtual void exec (void) final;
			virtual void configurationBegin(void) final;
			virtual void configurationEnd(void) final;
			void finish (const unsigned int ST0Flags);

			E5150::Floppy100* m_floppyToApply = nullptr;
			bool m_firstStep;
			public: Recalibrate(void);
		};

		class SenseInterruptStatus: public Command
		{
			virtual void configurationEnd(void) final;
			public: SenseInterruptStatus(void);
		};

		class Specify: public Command
		{
			virtual void configurationEnd(void) final;
			public: Specify(void);
		};

		class SenseDriveStatus: public Command
		{
			virtual void configurationEnd(void) final;
			public: SenseDriveStatus(void);
		};
		
		class Seek: public Command
		{
			public: Seek(void);
			private:
			virtual void exec (void) final;
			virtual void configurationBegin(void) final;
			virtual void configurationEnd(void) final;
			void execOnFloppyDrive (Floppy100& drive) const;

			private:
				void finish(const unsigned int st0Flags);

			private:
				bool m_direction;
				bool m_firstStep;
				Floppy100* m_floppyToApply = nullptr;
		};

		class Invalid: public Command
		{
			virtual void configurationEnd() final;
			public: Invalid(void);
		};
	}
}

#endif