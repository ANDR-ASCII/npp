#include "transport_packet_v2.h"

namespace Npp
{

TransportPacketV2::TransportPacketV2(const TransportHeaderV2& header, const std::vector<std::uint8_t>& data)
	: m_header(header)
	, m_data(data)
{
}

TransportPacketV2::TransportPacketV2(const TransportHeaderV2& header, std::vector<std::uint8_t>&& data)
	: m_header(header)
	, m_data(std::move(data))
{
}

TransportPacketVersion TransportPacketV2::version() const noexcept
{
	return TransportPacketVersion::TransportPacketVersion2;
}

std::uint16_t TransportPacketV2::sourcePort() const noexcept
{
	return m_header.sourcePort;
}

std::uint16_t TransportPacketV2::destinationPort() const noexcept
{
	return m_header.destinationPort;
}

std::uint16_t TransportPacketV2::dataSize() const noexcept
{
	return m_header.dataSize;
}

std::uint32_t TransportPacketV2::fragment() const noexcept
{
	return m_header.fragment;
}

std::uint8_t TransportPacketV2::mask() const noexcept
{
	return m_header.mask;
}

}