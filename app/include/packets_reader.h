#pragma once

#include "network_packet.h"

namespace Test
{

class PacketsReader
{
public:
	static std::vector<NetworkPacket> read(std::ifstream& stream);
};

}