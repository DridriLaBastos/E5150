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
				Command (const std::string& name = "", const unsigned int configurationWorldNumber=9, const unsigned int resultWorldNumber=7);

				virtual bool configure (const uint8_t data);

				//Return true when reading result is done
				std::pair<uint8_t,bool> readResult (void);

				virtual void exec (void);
				const std::string m_name;
				
				//TODO: I don't like having vectors here
				std::vector<uint8_t> m_configurationWords;
				std::vector<uint8_t> m_resultWords;
				unsigned int m_clockWait;
				unsigned int m_floppyDrive;
				unsigned int m_configurationStep;

				virtual void onConfigureBegin  (void);
				virtual void onConfigureFinish (void);
			
		};

		class ReadData: public Command
		{
			enum class STATUS
			{ LOADING_HEADS, READ_DATA };

			void loadHeads(void);
			virtual void exec () final;
			//virtual void onConfigureFinish(void) final;

			STATUS m_status { STATUS::LOADING_HEADS };
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
			virtual bool configure (const uint8_t data) final;
			virtual void exec (void) final;
			virtual void onConfigureBegin(void) final;
			virtual void onConfigureFinish(void) final;
			void finish (const unsigned int ST0Flags);

			E5150::Floppy100* m_floppyToApply = nullptr;
			bool m_firstStep;
			public: Recalibrate(void);
		};

		class SenseInterruptStatus: public Command
		{
			virtual void onConfigureFinish(void) final;
			virtual bool configure (const uint8_t data) final;
			public: SenseInterruptStatus(void);
		};

		class Specify: public Command
		{
			virtual void onConfigureFinish(void) final;
			virtual bool configure (const uint8_t data) final;
			public: Specify(void);
		};

		class SenseDriveStatus: public Command
		{
			virtual bool configure (const uint8_t data) final;
			virtual void onConfigureFinish(void) final;
			public: SenseDriveStatus(void);
		};
		
		class Seek: public Command
		{
			public: Seek(void);
			private:
			virtual void exec (void) final;
			virtual void onConfigureBegin(void) final;
			virtual void onConfigureFinish(void) final;
			virtual bool configure (const uint8_t data) final;
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
			virtual bool configure(const uint8_t data) final;
			public: Invalid(void);
		};
	}
}

#endif