#pragma once

namespace Test
{

enum class TransportPacketVersion
{
	TransportPacketVersion1,
	TransportPacketVersion2
};

class ITransportPacket
{
public:
	virtual TransportPacketVersion version() const noexcept = 0;
	virtual std::uint16_t sourcePort() const noexcept = 0;
	virtual std::uint16_t destinationPort() const noexcept = 0;
	virtual std::uint16_t dataSize() const noexcept = 0;
};

}