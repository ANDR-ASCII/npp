#include "transport_packet_v2.h"
#include "helpers.h"

namespace Test
{

TransportPacketV2::TransportPacketV2(const TransportHeaderV2& header, const std::vector<std::uint8_t>& data, std::uint16_t checksum)
	: m_header(header)
	, m_data(data)
	, m_checksum(checksum)
{
}

TransportPacketV2::TransportPacketV2(const TransportHeaderV2& header, std::vector<std::uint8_t>&& data, std::uint16_t checksum)
	: m_header(header)
	, m_data(std::move(data))
	, m_checksum(checksum)
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

std::uint16_t TransportPacketV2::checksum() const noexcept
{
	return m_checksum;
}

bool TransportPacketV2::hasBrokenChecksum() const noexcept
{
	std::uint64_t sum = 0;

	sum += Helpers::byteSum(m_header.sourcePort, Helpers::compileTimePow(2, 16));
	sum += Helpers::byteSum(m_header.destinationPort, Helpers::compileTimePow(2, 16));
	sum += Helpers::byteSum(m_header.dataSize, Helpers::compileTimePow(2, 16));
	sum += Helpers::byteSum(m_header.fragment, Helpers::compileTimePow(2, 16));
	sum += m_header.mask;

	for (std::size_t i = 0, sz = m_data.size(); i < sz; ++i)
	{
		sum += m_data[i] % Helpers::compileTimePow(2, 16);
	}

	const std::uint64_t calculatedChecksum = sum % Helpers::compileTimePow(2, 16);

	return checksum() != calculatedChecksum;
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