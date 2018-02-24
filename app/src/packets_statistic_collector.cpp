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

		const std::uint8_t* packetPointer = m_packetRawData.buffer->data() + m_packetRawData.startIndex;

		std::memcpy(&networkHeader.version, packetPointer + g_np_versionOffset, sizeof(std::uint8_t));

		assert(networkHeader.version == 1 || networkHeader.version == 2);

		if (networkHeader.version == 1)
		{
			std::memcpy(&networkHeader.sourceAddress.v1, packetPointer + g_np_sourceAddressOffset, sizeof(std::uint32_t));
			std::memcpy(&networkHeader.destinationAddress.v1, packetPointer + g_np_destinationAddressV1Offset, sizeof(std::uint32_t));
			std::memcpy(&networkHeader.protocol, packetPointer + g_np_protocolV1Offset, sizeof(std::uint8_t));
			std::memcpy(&networkHeader.dataSize, packetPointer + g_np_dataSizeV1Offset, sizeof(std::uint16_t));
			std::memcpy(&networkHeader.headerChecksum, packetPointer + g_np_headerChecksumV1Offset, sizeof(std::uint16_t));

			Helpers::fromBigEndian(networkHeader.sourceAddress.v1);
			Helpers::fromBigEndian(networkHeader.destinationAddress.v1);
			Helpers::fromBigEndian(networkHeader.dataSize);
			Helpers::fromBigEndian(networkHeader.headerChecksum);
		}
		else
		{
			std::memcpy(&networkHeader.sourceAddress.v2, packetPointer + g_np_sourceAddressOffset, sizeof(NetworkV2AddressType));
			std::memcpy(&networkHeader.destinationAddress.v2, packetPointer + g_np_destinationAddressV2Offset, sizeof(NetworkV2AddressType));
			std::memcpy(&networkHeader.protocol, packetPointer + g_np_protocolV2Offset, sizeof(std::uint8_t));
			std::memcpy(&networkHeader.dataSize, packetPointer + g_np_dataSizeV2Offset, sizeof(std::uint16_t));
			std::memcpy(&networkHeader.headerChecksum, packetPointer + g_np_headerChecksumV2Offset, sizeof(std::uint16_t));

			Helpers::fromBigEndian(networkHeader.sourceAddress.v2);
			Helpers::fromBigEndian(networkHeader.destinationAddress.v2);
			Helpers::fromBigEndian(networkHeader.dataSize);
			Helpers::fromBigEndian(networkHeader.headerChecksum);
		}

		std::vector<std::uint8_t> packetData;
		packetData.reserve(networkHeader.dataSize);

		const std::size_t dataOffset = networkHeader.version == 1 ? g_np_dataV1Offset : g_np_dataV2Offset;

		std::copy(
			m_packetRawData.buffer->begin() + m_packetRawData.startIndex + dataOffset,
			m_packetRawData.buffer->begin() + m_packetRawData.startIndex + dataOffset + networkHeader.dataSize,
			std::back_inserter(packetData)
		);

		std::shared_ptr<NetworkPacket> packet = std::make_shared<NetworkPacket>(NetworkPacketData{ networkHeader, std::move(packetData) });

		m_packetRawData.callback(std::move(packet));
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
	ThreadPool pool;

	std::cout << "Processing..." << std::endl;

	stream.seekg(0, std::ios::end);

	Buffer buffer;
	buffer.reserve(stream.tellg());

	stream.seekg(0, std::ios::beg);

	while (true)
	{
		const char ch = stream.get();
		
		if (stream.eof() && stream.fail())
		{
			break;
		}

		buffer.push_back(ch);
	}

	auto start = std::chrono::high_resolution_clock::now();

	std::vector<std::future<void>> futures;

	for (std::size_t i = 0, sz = buffer.size(); i < sz;)
	{
		assert(buffer[i] == 1 || buffer[i] == 2);

		const std::size_t dataSizeOffset = (buffer[i] == 1 ? g_np_dataSizeV1Offset : g_np_dataSizeV2Offset);
		const std::size_t dataOffset = (buffer[i] == 1 ? g_np_dataV1Offset : g_np_dataV2Offset);

		std::uint16_t dataSize = 0;

		std::memcpy(&dataSize, buffer.data() + i + dataSizeOffset, sizeof(std::uint16_t));
		Helpers::fromBigEndian(dataSize);

		const auto callback = [this](std::shared_ptr<NetworkPacket> packet)
		{
			add(std::move(packet));
		};

		// In this case main thread may be idle
		futures.push_back(pool.pushTask(CreatePacketTask(PacketRawData{ &buffer, i, callback })));

		//futures.push_back(std::async(CreatePacketTask(PacketRawData{ &buffer, i, callback })));

		i += dataOffset + dataSize;
	}

	std::for_each(futures.begin(), futures.end(), [](const std::future<void>& future) { future.wait(); });

	std::cout << "Done!" << std::endl;

	auto end = std::chrono::high_resolution_clock::now();

	std::cout << "Elapsed time ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl << std::endl;
}

bool PacketsStatisticCollector::isValid() const noexcept
{
	return m_isValid;
}

void PacketsStatisticCollector::add(std::shared_ptr<NetworkPacket> packet)
{
	if (packet->version() == 1)
	{
		++m_networkV1PacketsCount;

		std::lock_guard<std::mutex> locker(m_uniqueNetworkV1Addresses.mutex);
		m_uniqueNetworkV1Addresses.collection.insert(packet->sourceAddress().absoluteValue());
		m_uniqueNetworkV1Addresses.collection.insert(packet->destinationAddress().absoluteValue());
	}

	if (packet->version() == 2)
	{
		++m_networkV2PacketsCount;

		std::lock_guard<std::mutex> locker(m_uniqueNetworkV2Addresses.mutex);
		m_uniqueNetworkV2Addresses.collection.insert(packet->sourceAddress().absoluteValue());
		m_uniqueNetworkV2Addresses.collection.insert(packet->destinationAddress().absoluteValue());
	}

	if (packet->protocol() == 1)
	{
		++m_transportV1PacketsCount;

		if (packet->transportPacket()->hasBrokenChecksum())
		{
			++m_transportV1PacketsBrokenChecksumCount;
		}
	}

	if (packet->protocol() == 2)
	{
		++m_transportV2PacketsCount;

		if (packet->transportPacket()->hasBrokenChecksum())
		{
			++m_transportV2PacketsBrokenChecksumCount;
		}
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

std::size_t PacketsStatisticCollector::networkV1UniqueAddrsCount() const noexcept
{
	std::lock_guard<std::mutex> locker(m_uniqueNetworkV1Addresses.mutex);
	return m_uniqueNetworkV1Addresses.collection.size();
}

std::size_t PacketsStatisticCollector::networkV2UniqueAddrsCount() const noexcept
{
	std::lock_guard<std::mutex> locker(m_uniqueNetworkV2Addresses.mutex);
	return m_uniqueNetworkV2Addresses.collection.size();
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

int PacketsStatisticCollector::networkPacketsCount() const noexcept
{
	return m_networkV1PacketsCount + m_networkV2PacketsCount;
}

}