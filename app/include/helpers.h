#pragma once

namespace Test
{

class Helpers final
{
public:
	enum class ByteOrder
	{
		BigEndian,
		LittleEndian
	};

	static ByteOrder platrformByteOrder() noexcept
	{
		std::uint16_t number = 0x0001;

		return *reinterpret_cast<const std::uint8_t*>(&number) == 0 ?
			ByteOrder::BigEndian :
			ByteOrder::LittleEndian;
	}

	template <typename T>
	static void fromBigEndian(T& number)
	{
		std::uint8_t* bytePartsOfNumber = reinterpret_cast<std::uint8_t*>(&number);

		if (platrformByteOrder() == ByteOrder::BigEndian)
		{
			return;
		}

		for (int i = 0; i < sizeof(T) - i - 1; ++i)
		{
			std::swap(bytePartsOfNumber[i], bytePartsOfNumber[sizeof(T) - i - 1]);
		}
	}
};

}