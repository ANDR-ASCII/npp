#include "network_packet.h"
#include "transport_packet_v1.h"
#include "transport_packet_v2.h"

namespace Npp
{

NetworkPacket::NetworkPacket(const NetworkHeader& header, const std::vector<std::uint8_t>& data)
	: m_header(header)
	, m_data(data)
{
}

NetworkPacket::NetworkPacket(const NetworkHeader& header, std::vector<std::uint8_t>&& data)
	: m_header(header)
	, m_data(std::move(data))
{
}

std::uint8_t NetworkPacket::version() const noexcept
{
	return m_header.version;
}

AddressWrapper NetworkPacket::sourceAddress() const noexcept
{
	return version() == 1 ?
		AddressWrapper(m_header.sourceAddress.v1) :
		AddressWrapper(m_header.sourceAddress.v2);
}

AddressWrapper NetworkPacket::destinationAddress() const noexcept
{
	return version() == 1 ?
		AddressWrapper(m_header.destinationAddress.v1) :
		AddressWrapper(m_header.destinationAddress.v2);
}

std::uint8_t NetworkPacket::protocol() const noexcept
{
	return m_header.protocol;
}

std::uint16_t NetworkPacket::dataSize() const noexcept
{
	return m_header.dataSize;
}

std::uint16_t NetworkPacket::headerChecksum() const noexcept
{
	return m_header.headerChecksum;
}

std::shared_ptr<ITransportPacket> NetworkPacket::transportPacket() const
{
	std::shared_ptr<ITransportPacket> transportPacket;

	switch (m_header.protocol)
	{
		case 1:
		{
			transportPacket = std::make_shared<TransportPacketV1>(TransportHeaderV1{}, std::vector<std::uint8_t>{});
			break;
		}
		case 2:
		{
			transportPacket = std::make_shared<TransportPacketV2>(TransportHeaderV2{}, std::vector<std::uint8_t>{});
			break;
		}
		default:
		{
			assert(!"Invalid top level protocol version");
		}
	}

	return transportPacket;
}

}