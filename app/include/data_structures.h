#pragma once

namespace Test
{

using NetworkV2AddressType = std::uint8_t[6];

union Address
{
	std::uint32_t v1;
	NetworkV2AddressType v2;
};

// https://www.ibm.com/developerworks/library/pa-dalign/
// in article described the reason why structures aligned by default
// and fields are not in the right order.

struct NetworkHeader
{
	Address sourceAddress;
	Address destinationAddress;
	std::uint16_t dataSize;
	std::uint16_t headerChecksum;
	std::uint8_t version;
	std::uint8_t protocol;
};

struct TransportHeaderV1
{
	std::uint16_t sourcePort;
	std::uint16_t destinationPort;
	std::uint16_t dataSize;
};

struct TransportHeaderV2
{
	std::uint32_t fragment;
	std::uint16_t sourcePort;
	std::uint16_t destinationPort;
	std::uint16_t dataSize;
	std::uint8_t mask;
};

// np - network protocol
// tp - transport protocol

constexpr std::size_t g_np_versionOffset = 0;
constexpr std::size_t g_np_sourceAddressOffset = 1;
constexpr std::size_t g_np_destinationAddressV1Offset = 5;
constexpr std::size_t g_np_destinationAddressV2Offset = 7;
constexpr std::size_t g_np_protocolV1Offset = 9;
constexpr std::size_t g_np_protocolV2Offset = 13;
constexpr std::size_t g_np_dataSizeV1Offset = 10;
constexpr std::size_t g_np_dataSizeV2Offset = 14;
constexpr std::size_t g_np_headerChecksumV1Offset = 12;
constexpr std::size_t g_np_headerChecksumV2Offset = 16;
constexpr std::size_t g_np_dataV1Offset = 14;
constexpr std::size_t g_np_dataV2Offset = 18;

constexpr std::size_t g_tp_sourcePortOffset = 0;
constexpr std::size_t g_tp_destinationPortOffset = 2;
constexpr std::size_t g_tp_fragmentOffset = 4;
constexpr std::size_t g_tp_maskOffset = 8;
constexpr std::size_t g_tp_dataSizeV1Offset = 4;
constexpr std::size_t g_tp_dataSizeV2Offset = 9;
constexpr std::size_t g_tp_dataV1Offset = 6;
constexpr std::size_t g_tp_dataV2Offset = 11;

constexpr std::size_t g_tp_headerSizeV1 = 6;
constexpr std::size_t g_tp_headerSizeV2 = 11;

}