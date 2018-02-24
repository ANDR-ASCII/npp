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

	int networkPacketsCount() const noexcept;

	int networkV1PacketsCount() const noexcept;
	int networkV2PacketsCount() const noexcept;

	std::size_t networkV1UniqueAddrsCount() const noexcept;
	std::size_t networkV2UniqueAddrsCount() const noexcept;

	int transportV1PacketsCount() const noexcept;
	int transportV2PacketsCount() const noexcept;

	int transportV1PacketsBrokenChecksumCount() const noexcept;
	int transportV2PacketsBrokenChecksumCount() const noexcept;

	int transportV1UniquePortsCount() const noexcept;
	int transportV2UniquePortsCount() const noexcept;

private:
	void collect(std::ifstream& stream);

private:
	template <typename T, typename Hasher = std::hash<T>, typename Comparator = std::equal_to<T>>
	struct HashTable final
	{
		mutable std::mutex mutex;
		std::unordered_set<T, Hasher, Comparator> collection;
	};

	bool m_isValid;

	std::atomic_int m_networkV1PacketsCount;
	std::atomic_int m_networkV2PacketsCount;

	std::atomic_int m_transportV1PacketsCount;
	std::atomic_int m_transportV2PacketsCount;

	std::atomic_int m_transportV1PacketsBrokenChecksumCount;
	std::atomic_int m_transportV2PacketsBrokenChecksumCount;

	std::atomic_int m_transportV1UniquePortsCount;
	std::atomic_int m_transportV2UniquePortsCount;

	HashTable<std::uint64_t> m_uniqueNetworkV1Addresses;
	HashTable<std::uint64_t> m_uniqueNetworkV2Addresses;
};

}