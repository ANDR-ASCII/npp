#include "transport_packet_v1.h"
#include "helpers.h"

namespace Test
{

TransportPacketV1::TransportPacketV1(const TransportHeaderV1& header, const std::vector<std::uint8_t>& data, std::uint16_t checksum)
	: m_header(header)
	, m_data(data)
	, m_checksum(checksum)
{
}

TransportPacketV1::TransportPacketV1(const TransportHeaderV1& header, std::vector<std::uint8_t>&& data, std::uint16_t checksum)
	: m_header(header)
	, m_data(std::move(data))
	, m_checksum(checksum)
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

std::uint16_t TransportPacketV1::checksum() const noexcept
{
	return m_checksum;
}

bool TransportPacketV1::hasBrokenChecksum() const noexcept
{
	std::uint64_t sum = 0;

	sum += Helpers::byteSum(m_header.sourcePort, Helpers::compileTimePow(2, 16));
	sum += Helpers::byteSum(m_header.destinationPort, Helpers::compileTimePow(2, 16));
	sum += Helpers::byteSum(m_header.dataSize, Helpers::compileTimePow(2, 16));

	for (std::size_t i = 0, sz = m_data.size(); i < sz; ++i)
	{
		sum += m_data[i] % Helpers::compileTimePow(2, 16);
	}

	const std::uint64_t calculatedChecksum = sum % Helpers::compileTimePow(2, 16);

	return checksum() != calculatedChecksum;
}

}