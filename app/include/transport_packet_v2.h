#pragma once

#include "data_structures.h"
#include "itransport_packet.h"

namespace Test
{

class TransportPacketV2 : public ITransportPacket
{
public:
	TransportPacketV2(const TransportHeaderV2& header, const std::vector<std::uint8_t>& data, std::uint16_t checksum);
	TransportPacketV2(const TransportHeaderV2& header, std::vector<std::uint8_t>&& data, std::uint16_t checksum);

	virtual TransportPacketVersion version() const noexcept override;
	virtual std::uint16_t sourcePort() const noexcept override;
	virtual std::uint16_t destinationPort() const noexcept override;
	virtual std::uint16_t dataSize() const noexcept override;
	virtual std::uint16_t checksum() const noexcept override;
	virtual bool hasBrokenChecksum() const noexcept override;
	std::uint32_t fragment() const noexcept;
	std::uint8_t mask() const noexcept;

private:
	TransportHeaderV2 m_header;
	std::vector<std::uint8_t> m_data;
	std::uint16_t m_checksum;
};

}