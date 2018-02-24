#pragma once

#include "data_structures.h"

namespace Test
{

class AddressWrapper final
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
	std::uint64_t absoluteValue() const;

	friend bool operator==(const AddressWrapper& lhs, const AddressWrapper& rhs);
	friend bool operator!=(const AddressWrapper& lhs, const AddressWrapper& rhs);

private:
	Address m_addr;
	Version m_version;
};

}