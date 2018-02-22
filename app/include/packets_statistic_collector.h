#pragma once

#include "network_packet.h"

namespace Test
{

class PacketsStatisticCollector final
{
public:
	PacketsStatisticCollector(const std::string& filepath);
	
	void add(std::shared_ptr<NetworkPacket> packet);

	bool isValid() const noexcept;

	int networkV1PacketsCount() const noexcept;
	int networkV2PacketsCount() const noexcept;

	int networkV1UniqueAddrsCount() const noexcept;
	int networkV2UniqueAddrsCount() const noexcept;

	int transportV1PacketsCount() const noexcept;
	int transportV2PacketsCount() const noexcept;

	int transportV1PacketsBrokenChecksumCount() const noexcept;
	int transportV2PacketsBrokenChecksumCount() const noexcept;

	int transportV1UniquePortsCount() const noexcept;
	int transportV2UniquePortsCount() const noexcept;

private:
	void collect(std::ifstream& stream);

private:
	bool m_isValid;

	mutable std::mutex m_mutex;
	std::vector<std::shared_ptr<NetworkPacket>> m_allPackets;

	std::atomic_int m_networkV1PacketsCount;
	std::atomic_int m_networkV2PacketsCount;

	std::atomic_int m_networkV1UniqueAddrsCount;
	std::atomic_int m_networkV2UniqueAddrsCount;

	std::atomic_int m_transportV1PacketsCount;
	std::atomic_int m_transportV2PacketsCount;

	std::atomic_int m_transportV1PacketsBrokenChecksumCount;
	std::atomic_int m_transportV2PacketsBrokenChecksumCount;

	std::atomic_int m_transportV1UniquePortsCount;
	std::atomic_int m_transportV2UniquePortsCount;
};

}