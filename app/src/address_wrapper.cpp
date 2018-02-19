#include "address_wrapper.h"

namespace Npp
{

AddressWrapper::AddressWrapper(std::uint32_t addr)
	: m_version(Version1)
{
	m_addr.v1 = addr;
}

AddressWrapper::AddressWrapper(const NetworkV2AddressType& addr)
	: m_version(Version2)
{
	std::memcpy(m_addr.v2, addr, sizeof(NetworkV2AddressType));
}

std::string AddressWrapper::toString() const
{
	std::string address;

	switch (m_version)
	{
	case Version1:
	{
		for (unsigned int i = 0; i < sizeof(std::uint32_t); ++i)
		{
			address += std::to_string(static_cast<std::uint8_t>(m_addr.v1 >> ((sizeof(std::uint32_t) - i - 1) * 8)));

			if (i != sizeof(std::uint32_t) - 1)
			{
				address += ".";
			}
		}

		return address;
	}
	case Version2:
	{
		for (unsigned int i = 0; i < sizeof(NetworkV2AddressType); ++i)
		{
			address += std::to_string(m_addr.v2[i]);

			if (i != sizeof(NetworkV2AddressType) - 1)
			{
				address += ".";
			}
		}

		return address;
	}
	default:
	{
		assert(!"Undefined version");
	}
	}

	return std::string();
}

}