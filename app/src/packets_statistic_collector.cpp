#include "packets_statistic_collector.h"
#include "helpers.h"
#include "thread_pool.h"

#pragma comment(lib, "ws2_32.lib")

namespace
{

using namespace Test;

using ResultStatCollectorCallback = std::function<void(std::shared_ptr<NetworkPacket>)>;
using Buffer = std::vector<std::uint8_t>;

struct PacketRawData
{
	const Buffer* buffer;
	std::size_t startIndex;
	ResultStatCollectorCallback callback;
};

class CreatePacketTask
{
public:
	CreatePacketTask(const PacketRawData& packetRawData)
		: m_packetRawData(packetRawData)
	{
	}

	void operator()() const
	{
		NetworkHeader networkHeader;

		std::memcpy(&networkHeader.version, m_packetRawData.buffer->data() + m_packetRawData.startIndex + g_np_versionOffset, sizeof(std::uint8_t));

		assert(networkHeader.version == 1 || networkHeader.version == 2);

		if (networkHeader.version == 1)
		{
			std::memcpy(&networkHeader.sourceAddress.v1, m_packetRawData.buffer->data() + m_packetRawData.startIndex + g_np_sourceAddressOffset, sizeof(std::uint32_t));
			std::memcpy(&networkHeader.destinationAddress.v1, m_packetRawData.buffer->data() + m_packetRawData.startIndex + g_np_destinationAddressV1Offset, sizeof(std::uint32_t));
			std::memcpy(&networkHeader.protocol, m_packetRawData.buffer->data() + m_packetRawData.startIndex + g_np_protocolV1Offset, sizeof(std::uint8_t));
			std::memcpy(&networkHeader.dataSize, m_packetRawData.buffer->data() + m_packetRawData.startIndex + g_np_dataSizeV1Offset, sizeof(std::uint16_t));
			std::memcpy(&networkHeader.headerChecksum, m_packetRawData.buffer->data() + m_packetRawData.startIndex + g_np_headerChecksumV1Offset, sizeof(std::uint16_t));

			Helpers::fromBigEndian(networkHeader.sourceAddress.v1);
			Helpers::fromBigEndian(networkHeader.destinationAddress.v1);
			Helpers::fromBigEndian(networkHeader.dataSize);
			Helpers::fromBigEndian(networkHeader.headerChecksum);
		}
		else
		{
			std::memcpy(&networkHeader.sourceAddress.v2, m_packetRawData.buffer->data() + m_packetRawData.startIndex + g_np_sourceAddressOffset, sizeof(NetworkV2AddressType));
			std::memcpy(&networkHeader.destinationAddress.v2, m_packetRawData.buffer->data() + m_packetRawData.startIndex + g_np_destinationAddressV2Offset, sizeof(NetworkV2AddressType));
			std::memcpy(&networkHeader.protocol, m_packetRawData.buffer->data() + m_packetRawData.startIndex + g_np_protocolV2Offset, sizeof(std::uint8_t));
			std::memcpy(&networkHeader.dataSize, m_packetRawData.buffer->data() + m_packetRawData.startIndex + g_np_dataSizeV2Offset, sizeof(std::uint16_t));
			std::memcpy(&networkHeader.headerChecksum, m_packetRawData.buffer->data() + m_packetRawData.startIndex + g_np_headerChecksumV2Offset, sizeof(std::uint16_t));

			Helpers::fromBigEndian(networkHeader.sourceAddress.v2);
			Helpers::fromBigEndian(networkHeader.destinationAddress.v2);
			Helpers::fromBigEndian(networkHeader.dataSize);
			Helpers::fromBigEndian(networkHeader.headerChecksum);
		}

		std::vector<std::uint8_t> packetData;
		packetData.reserve(networkHeader.dataSize);

		auto backInserter = std::back_inserter(packetData);

		std::copy(
			m_packetRawData.buffer->begin() + m_packetRawData.startIndex,
			m_packetRawData.buffer->begin() + m_packetRawData.startIndex + networkHeader.dataSize,
			backInserter
		);

		m_packetRawData.callback(std::make_shared<NetworkPacket>(NetworkPacketData{ networkHeader, std::move(packetData) }));
	}

private:
	PacketRawData m_packetRawData;
};

}

namespace Test
{

PacketsStatisticCollector::PacketsStatisticCollector(const std::string& filepath)
	: m_isValid(false)
	, m_networkV1PacketsCount(0)
	, m_networkV2PacketsCount(0)
	, m_networkV1UniqueAddrsCount(0)
	, m_networkV2UniqueAddrsCount(0)
	, m_transportV1PacketsCount(0)
	, m_transportV2PacketsCount(0)
	, m_transportV1PacketsBrokenChecksumCount(0)
	, m_transportV2PacketsBrokenChecksumCount(0)
	, m_transportV1UniquePortsCount(0)
	, m_transportV2UniquePortsCount(0)
{
	std::ifstream stream(filepath, std::ios_base::binary);

	if (stream)
	{
		collect(stream);

		m_isValid = true;
	}
}

void PacketsStatisticCollector::collect(std::ifstream& stream)
{
	std::cout << "Processing..." << std::endl;

	// ThreadPool pool;

	stream.seekg(0, std::ios::end);

	Buffer buffer;
	buffer.reserve(stream.tellg());

	stream.seekg(0, std::ios::beg);

	while (!stream.eof())
	{
		buffer.push_back(stream.get());
	}

	std::vector<std::future<void>> futures;

	for (std::size_t i = 0, sz = buffer.size(); i < sz;)
	{
		assert(buffer[i] == 1 || buffer[i] == 2);

		const std::size_t dataSizeOffset = (buffer[i] == 1 ? g_np_dataSizeV1Offset : g_np_dataSizeV2Offset);
		const std::size_t dataOffset = (buffer[i] == 1 ? g_np_dataV1Offset : g_np_dataV2Offset);

		const std::uint16_t dataSize = ntohs(*reinterpret_cast<std::uint16_t*>(buffer.data() + i + dataSizeOffset));

		const auto callback = [this](std::shared_ptr<NetworkPacket> packet)
		{
			add(std::move(packet));
		};

		// In this case main thread may be idle
		// futures.push_back(pool.pushTask(CreatePacketTask(PacketRawData{ &buffer, i, callback })));

		futures.push_back(std::async(CreatePacketTask(PacketRawData{ &buffer, i, callback })));

		i += dataOffset + dataSize;

		if (i == buffer.size() - 1)
		{
			break;
		}
	}

	std::for_each(futures.begin(), futures.end(), [](const std::future<void>& future) { future.wait(); });

	std::cout << "Done!" << std::endl;
}

bool PacketsStatisticCollector::isValid() const noexcept
{
	return m_isValid;
}

void PacketsStatisticCollector::add(std::shared_ptr<NetworkPacket> packet)
{
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		m_allPackets.emplace_back(std::move(packet));
	}

	if (packet->version() == 1)
	{
		++m_networkV1PacketsCount;
	}

	if (packet->version() == 2)
	{
		++m_networkV2PacketsCount;
	}
}

int PacketsStatisticCollector::networkV1PacketsCount() const noexcept
{
	return m_networkV1PacketsCount;
}

int PacketsStatisticCollector::networkV2PacketsCount() const noexcept
{
	return m_networkV2PacketsCount;
}

int PacketsStatisticCollector::networkV1UniqueAddrsCount() const noexcept
{
	return m_networkV1UniqueAddrsCount;
}

int PacketsStatisticCollector::networkV2UniqueAddrsCount() const noexcept
{
	return m_networkV2UniqueAddrsCount;
}

int PacketsStatisticCollector::transportV1PacketsCount() const noexcept
{
	return m_transportV1PacketsCount;
}

int PacketsStatisticCollector::transportV2PacketsCount() const noexcept
{
	return m_transportV2PacketsCount;
}

int PacketsStatisticCollector::transportV1PacketsBrokenChecksumCount() const noexcept
{
	return m_transportV1PacketsBrokenChecksumCount;
}

int PacketsStatisticCollector::transportV2PacketsBrokenChecksumCount() const noexcept
{
	return m_transportV2PacketsBrokenChecksumCount;
}

int PacketsStatisticCollector::transportV1UniquePortsCount() const noexcept
{
	return m_transportV1UniquePortsCount;
}

int PacketsStatisticCollector::transportV2UniquePortsCount() const noexcept
{
	return m_transportV2UniquePortsCount;
}

}