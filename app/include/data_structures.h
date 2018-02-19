#pragma once

namespace Npp
{

using NetworkV2AddressType = std::uint8_t[6];

union Address
{
	std::uint32_t v1;
	NetworkV2AddressType v2;
};

struct NetworkHeader
{
	std::uint8_t version;
	Address sourceAddress;
	Address destinationAddress;
	std::uint8_t protocol;
	std::uint16_t dataSize;
	std::uint16_t headerChecksum;
};

struct TransportHeaderV1
{
	std::uint16_t sourcePort;
	std::uint16_t destinationPort;
	std::uint16_t dataSize;
};

struct TransportHeaderV2
{
	std::uint16_t sourcePort;
	std::uint16_t destinationPort;
	std::uint16_t dataSize;
	std::uint32_t fragment;
	std::uint8_t mask;
};

}