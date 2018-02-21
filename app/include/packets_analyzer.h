#pragma once

#include "network_packet.h"

namespace Test
{

class PacketsStatisticCollector final
{
public:
	PacketsStatisticCollector(std::vector<NetworkPacket>);

private:
	std::vector<NetworkPacket> m_packets;
};

}