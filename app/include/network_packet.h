#pragma once

#include "data_structures.h"
#include "address_wrapper.h"
#include "itransport_packet.h"

namespace Npp
{

class NetworkPacket
{
public:
	NetworkPacket(const NetworkHeader& header, const std::vector<std::uint8_t>& data);
	NetworkPacket(const NetworkHeader& header, std::vector<std::uint8_t>&& data);

	std::uint8_t version() const noexcept;
	AddressWrapper sourceAddress() const noexcept;
	AddressWrapper destinationAddress() const noexcept;
	std::uint8_t protocol() const noexcept;
	std::uint16_t dataSize() const noexcept;
	std::uint16_t headerChecksum() const noexcept;
	std::shared_ptr<ITransportPacket> transportPacket() const;

private:
	NetworkHeader m_header;
	std::vector<std::uint8_t> m_data;
};

}