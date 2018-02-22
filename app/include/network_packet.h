#pragma once

#include "data_structures.h"
#include "address_wrapper.h"
#include "itransport_packet.h"

namespace Test
{

struct NetworkPacketData
{
	NetworkHeader header;
	std::vector<std::uint8_t> data;
};

class NetworkPacket final
{
public:
	NetworkPacket();
	NetworkPacket(const NetworkPacketData& data);
	NetworkPacket(NetworkPacketData&& data);

	bool isValid() const noexcept;
	std::uint8_t version() const noexcept;
	AddressWrapper sourceAddress() const noexcept;
	AddressWrapper destinationAddress() const noexcept;
	std::uint8_t protocol() const noexcept;
	std::uint16_t dataSize() const noexcept;
	std::uint16_t headerChecksum() const noexcept;
	std::shared_ptr<ITransportPacket> transportPacket() const;

private:
	bool m_isValid;
	NetworkPacketData m_networkPacketData;
};

}