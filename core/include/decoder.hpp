#ifndef __DECODER_HPP__
#define __DECODER_HPP__

#include "decoderHelper.hpp"

namespace E5150::I8086
{
	class Decoder
	{
		public:
			struct DecodedInstruction
			{
				DECODER::INSTRUCTION_NAME name;
				DECODER::OPERAND srcOperand;
				DECODER::OPERAND dstOperand;
				bool wordOperand;

				DECODER::REGISTER_NAME srcRegName;
				DECODER::REGISTER_NAME dstRegName;

				unsigned int srcUImmediate;
						 int dstSImmediate;
			};

		public:
			Decoder(void);
		
		public:
			bool feed (const uint8_t data);

			template<typename DATA_TYPE>
			void writeCPUReg(const DECODER::REGISTER_NAME reg, const DATA_TYPE data);

			template<typename DATA_TYPE>
			DATA_TYPE readCPUReg(const DECODER::REGISTER_NAME reg);
		
		public:
			DecodedInstruction decodedInstruction;
		private:
	};
}

#endif