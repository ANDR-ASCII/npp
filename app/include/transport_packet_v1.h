#pragma once

#include "data_structures.h"
#include "itransport_packet.h"

namespace Npp
{

class TransportPacketV1 : public ITransportPacket
{
public:
	TransportPacketV1(const TransportHeaderV1& header, const std::vector<std::uint8_t>& data);
	TransportPacketV1(const TransportHeaderV1& header, std::vector<std::uint8_t>&& data);

	virtual TransportPacketVersion version() const noexcept override;
	virtual std::uint16_t sourcePort() const noexcept override;
	virtual std::uint16_t destinationPort() const noexcept override;
	virtual std::uint16_t dataSize() const noexcept override;

private:
	TransportHeaderV1 m_header;
	std::vector<std::uint8_t> m_data;
	std::uint16_t m_checksum;
};

}