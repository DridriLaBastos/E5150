#ifndef __RAM_HPP__
#define __RAM_HPP__

namespace E5150
{
	class RAM
	{
	public:
		RAM(void);

	public:
		uint8_t Read(const unsigned int address) const;
		void Write(const unsigned int address, const uint8_t data);
		void LoadFromFile(const std::filesystem::path path, size_t startPos);

	private:
		std::unique_ptr<uint8_t> m_ram;

	};
}

#endif
