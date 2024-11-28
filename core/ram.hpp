#ifndef __RAM_HPP__
#define __RAM_HPP__

namespace E5150
{
	class RAM
	{
	public:
		RAM(void);

	public:
		uint8_t ReadByte(unsigned int address) const;
		uint16_t ReadWord(unsigned int address) const;
		void WriteByte(unsigned int address, uint8_t data);
		void WriteWord(unsigned int address, uint16_t data);
		void LoadFromFile(std::filesystem::path path, size_t startPos);

	private:
		std::unique_ptr<uint8_t> m_ram;

	};
}

#endif
