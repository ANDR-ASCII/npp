#include "network_packet.h"
#include "transport_packet_v1.h"
#include "transport_packet_v2.h"
#include "helpers.h"

namespace Test
{

NetworkPacket::NetworkPacket(const NetworkPacketData& data)
	: m_isValid(true)
	, m_networkPacketData(data)
{
	parseEmbeddedPacket();
}

NetworkPacket::NetworkPacket(NetworkPacketData&& data)
	: m_isValid(true)
	, m_networkPacketData(data)
{
	parseEmbeddedPacket();
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

ITransportPacket* NetworkPacket::transportPacket() const
{
	return m_embeddedTransportPacket.get();
}

void NetworkPacket::parseEmbeddedPacket()
{
	std::vector<std::uint8_t> transportPacketData;

	switch (m_networkPacketData.header.protocol)
	{
		case 1:
		{
			TransportHeaderV1 transportPacketHeader;

			const std::uint8_t* underlyingDataPointer = m_networkPacketData.data.data();

			std::memcpy(&transportPacketHeader.sourcePort, underlyingDataPointer + g_tp_sourcePortOffset, sizeof(std::uint16_t));
			std::memcpy(&transportPacketHeader.destinationPort, underlyingDataPointer + g_tp_destinationPortOffset, sizeof(std::uint16_t));
			std::memcpy(&transportPacketHeader.dataSize, underlyingDataPointer + g_tp_dataSizeV1Offset, sizeof(std::uint16_t));

			Helpers::fromBigEndian(transportPacketHeader.sourcePort);
			Helpers::fromBigEndian(transportPacketHeader.destinationPort);
			Helpers::fromBigEndian(transportPacketHeader.dataSize);

			std::uint16_t checksum = 0;

			//
			// what should we do if "transportPacketHeader.dataSize" is greater than packet itself???
			// in this case checksum guarantee will be invalid
			//
			if(g_tp_headerSizeV1 + transportPacketHeader.dataSize + sizeof(std::uint16_t) == m_networkPacketData.data.size())
			{
				transportPacketData.reserve(transportPacketHeader.dataSize);

				std::memcpy(&checksum, underlyingDataPointer + transportPacketHeader.dataSize + g_tp_dataV1Offset, sizeof(std::uint16_t));
				Helpers::fromBigEndian(checksum);

				const auto transportDataIterator = m_networkPacketData.data.begin() + g_tp_dataV1Offset;
				std::copy(transportDataIterator, m_networkPacketData.data.end(), std::back_inserter(transportPacketData));
			}

			m_embeddedTransportPacket = std::make_unique<TransportPacketV1>(transportPacketHeader, std::move(transportPacketData), checksum);

			break;
		}
		case 2:
		{
			TransportHeaderV2 transportPacketHeader;

			const std::uint8_t* underlyingDataPointer = m_networkPacketData.data.data();
			
			std::memcpy(&transportPacketHeader.sourcePort, underlyingDataPointer + g_tp_sourcePortOffset, sizeof(std::uint16_t));
			std::memcpy(&transportPacketHeader.destinationPort, underlyingDataPointer + g_tp_destinationPortOffset, sizeof(std::uint16_t));
			std::memcpy(&transportPacketHeader.fragment, underlyingDataPointer + g_tp_fragmentOffset, sizeof(std::uint32_t));
			std::memcpy(&transportPacketHeader.mask, underlyingDataPointer + g_tp_maskOffset, sizeof(std::uint8_t));
			std::memcpy(&transportPacketHeader.dataSize, underlyingDataPointer + g_tp_dataSizeV2Offset, sizeof(std::uint16_t));

			Helpers::fromBigEndian(transportPacketHeader.sourcePort);
			Helpers::fromBigEndian(transportPacketHeader.destinationPort);
			Helpers::fromBigEndian(transportPacketHeader.fragment);
			Helpers::fromBigEndian(transportPacketHeader.dataSize);

			std::uint16_t checksum = 0;

			//
			// what should we do if "transportPacketHeader.dataSize" is greater than packet itself???
			// in this case checksum guarantee will be invalid
			//
			if(g_tp_headerSizeV2 + transportPacketHeader.dataSize + sizeof(std::uint16_t) == m_networkPacketData.data.size())
			{
				transportPacketData.reserve(transportPacketHeader.dataSize);

				std::memcpy(&checksum, underlyingDataPointer + transportPacketHeader.dataSize + g_tp_dataV2Offset, sizeof(std::uint16_t));
				Helpers::fromBigEndian(checksum);

				const auto transportDataIterator = m_networkPacketData.data.begin() + g_tp_dataV2Offset;
				std::copy(transportDataIterator, m_networkPacketData.data.end(), std::back_inserter(transportPacketData));
			}

			m_embeddedTransportPacket = std::make_unique<TransportPacketV2>(transportPacketHeader, std::move(transportPacketData), checksum);

			break;
		}
		default:
		{
			assert(!"Invalid top level protocol version");
		}
	}
}

}