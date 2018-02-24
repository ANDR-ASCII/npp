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
		const std::uint16_t number = 0x0001;

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

	template <typename T>
	static std::uint64_t byteSum(const T& object, int mod)
	{
		std::uint64_t sum = 0;

		const std::uint8_t* bytes = reinterpret_cast<const std::uint8_t*>(&object);

		for (int i = 0; i < sizeof(T); ++i)
		{
			sum += bytes[i] % mod;
		}

		return sum;
	}

	static constexpr auto compileTimePow(int n, int degree)
	{
		if (!degree)
		{
			return 1;
		}

		return n * compileTimePow(n, degree - 1);
	}
};

}