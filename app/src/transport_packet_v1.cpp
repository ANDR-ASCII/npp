#include "transport_packet_v1.h"

namespace Npp
{

TransportPacketV1::TransportPacketV1(const TransportHeaderV1& header, const std::vector<std::uint8_t>& data)
	: m_header(header)
	, m_data(data)
{
}

TransportPacketV1::TransportPacketV1(const TransportHeaderV1& header, std::vector<std::uint8_t>&& data)
	: m_header(header)
	, m_data(std::move(data))
{
}

TransportPacketVersion TransportPacketV1::version() const noexcept
{
	return TransportPacketVersion::TransportPacketVersion1;
}

std::uint16_t TransportPacketV1::sourcePort() const noexcept
{
	return m_header.sourcePort;
}

std::uint16_t TransportPacketV1::destinationPort() const noexcept
{
	return m_header.destinationPort;
}

std::uint16_t TransportPacketV1::dataSize() const noexcept
{
	return m_header.dataSize;
}

}