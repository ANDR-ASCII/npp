#pragma once

#include "data_structures.h"

namespace Test
{

class AddressWrapper
{
private:
	enum Version
	{
		Version1,
		Version2
	};

public:
	AddressWrapper(std::uint32_t addr);
	AddressWrapper(const NetworkV2AddressType& addr);

	std::string toString() const;

private:
	Address m_addr;
	Version m_version;
};

}