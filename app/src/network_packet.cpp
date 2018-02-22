#include "network_packet.h"
#include "transport_packet_v1.h"
#include "transport_packet_v2.h"

namespace Test
{

NetworkPacket::NetworkPacket(const NetworkPacketData& data)
	: m_isValid(true)
	, m_networkPacketData(data)
{
}

NetworkPacket::NetworkPacket(NetworkPacketData&& data)
	: m_isValid(true)
	, m_networkPacketData(data)
{
}

NetworkPacket::NetworkPacket()
	: m_isValid(false)
{
}

bool NetworkPacket::isValid() const noexcept
{
	return m_isValid;
}

std::uint8_t NetworkPacket::version() const noexcept
{
	return m_networkPacketData.header.version;
}

AddressWrapper NetworkPacket::sourceAddress() const noexcept
{
	return m_networkPacketData.header.version == 1 ?
		AddressWrapper(m_networkPacketData.header.sourceAddress.v1) :
		AddressWrapper(m_networkPacketData.header.sourceAddress.v2);
}

AddressWrapper NetworkPacket::destinationAddress() const noexcept
{
	return m_networkPacketData.header.version == 1 ?
		AddressWrapper(m_networkPacketData.header.destinationAddress.v1) :
		AddressWrapper(m_networkPacketData.header.destinationAddress.v2);
}

std::uint8_t NetworkPacket::protocol() const noexcept
{
	return m_networkPacketData.header.protocol;
}

std::uint16_t NetworkPacket::dataSize() const noexcept
{
	return m_networkPacketData.header.dataSize;
}

std::uint16_t NetworkPacket::headerChecksum() const noexcept
{
	return m_networkPacketData.header.headerChecksum;
}

std::shared_ptr<ITransportPacket> NetworkPacket::transportPacket() const
{
	std::shared_ptr<ITransportPacket> transportPacket;

	std::vector<std::uint8_t> transportPacketData;
	transportPacketData.reserve(m_networkPacketData.header.dataSize);

	switch (m_networkPacketData.header.protocol)
	{
		case 1:
		{
			TransportHeaderV1 transportPacketHeader;

			const std::uint8_t* underlyingDataPointer = m_networkPacketData.data.data();

			transportPacketHeader.sourcePort = *reinterpret_cast<const std::uint16_t*>(underlyingDataPointer);
			transportPacketHeader.destinationPort = *(reinterpret_cast<const std::uint16_t*>(underlyingDataPointer) + 1);
			transportPacketHeader.dataSize = *(reinterpret_cast<const std::uint16_t*>(underlyingDataPointer) + 2);

			// 
			// I don't sure about can I here rely to "sizeof(TransportHeaderV1)"
			// because struct memory alignment is implementation defined
			// in common cases structs aligns by largest member
			//
			const auto transportDataIterator = m_networkPacketData.data.begin() + sizeof(std::uint16_t) * 3;

			std::uninitialized_copy(transportDataIterator, m_networkPacketData.data.end(), transportPacketData.begin());

			transportPacket = std::make_shared<TransportPacketV1>(transportPacketHeader, transportPacketData);

			break;
		}
		case 2:
		{
			TransportHeaderV2 transportPacketHeader;

			const std::uint8_t* underlyingDataPointer = m_networkPacketData.data.data();

			transportPacketHeader.sourcePort = *reinterpret_cast<const std::uint16_t*>(underlyingDataPointer);
			transportPacketHeader.destinationPort = *reinterpret_cast<const std::uint16_t*>(underlyingDataPointer + 2);
			transportPacketHeader.fragment = *reinterpret_cast<const std::uint32_t*>(underlyingDataPointer + 4);
			transportPacketHeader.mask = *underlyingDataPointer + 8;
			transportPacketHeader.dataSize = *reinterpret_cast<const std::uint16_t*>(underlyingDataPointer + 10);

			// 
			// I don't sure about can I here rely to "sizeof(TransportHeaderV2)"
			// because struct memory alignment is implementation defined
			// in common cases structs aligns by largest member
			//
			const auto transportDataIterator = m_networkPacketData.data.begin() + sizeof(std::uint16_t) * 3;

			std::uninitialized_copy(transportDataIterator, m_networkPacketData.data.end(), transportPacketData.begin());

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