#include "packets_reader.h"
#include "helpers.h"
#include "thread_pool.h"

#pragma comment(lib, "ws2_32.lib")

namespace
{

constexpr std::size_t s_versionOffset = 0;

constexpr std::size_t s_sourceAddressOffset = 1;

constexpr std::size_t s_destinationAddressV1Offset = 5;
constexpr std::size_t s_destinationAddressV2Offset = 7;

constexpr std::size_t s_protocolV1Offset = 9;
constexpr std::size_t s_protocolV2Offset = 13;

constexpr std::size_t s_dataSizeV1Offset = 10;
constexpr std::size_t s_dataSizeV2Offset = 14;

constexpr std::size_t s_headerChecksumV1Offset = 12;
constexpr std::size_t s_headerChecksumV2Offset = 16;

constexpr std::size_t s_dataV1Offset = 14;
constexpr std::size_t s_dataV2Offset = 18;


struct PacketRawData
{
	const std::vector<std::uint8_t>* buffer;
	std::size_t startIndex;
};

std::mutex mutex;
std::atomic_int counter = 0;

class CreatePacketTask
{
public:
	CreatePacketTask(const PacketRawData& packetRawData)
		: m_packetRawData(packetRawData)
	{
	}

	Test::NetworkPacket operator()() const
	{
		Test::NetworkHeader networkHeader;

		std::memcpy(&networkHeader.version, m_packetRawData.buffer->data() + m_packetRawData.startIndex + s_versionOffset, sizeof(std::uint8_t));

		assert(networkHeader.version == 1 || networkHeader.version == 2);

		if (networkHeader.version == 1)
		{
			std::memcpy(&networkHeader.sourceAddress.v1, m_packetRawData.buffer->data() + m_packetRawData.startIndex + s_sourceAddressOffset, sizeof(std::uint32_t));
			std::memcpy(&networkHeader.destinationAddress.v1, m_packetRawData.buffer->data() + m_packetRawData.startIndex + s_destinationAddressV1Offset, sizeof(std::uint32_t));
			std::memcpy(&networkHeader.protocol, m_packetRawData.buffer->data() + m_packetRawData.startIndex + s_protocolV1Offset, sizeof(std::uint8_t));
			std::memcpy(&networkHeader.dataSize, m_packetRawData.buffer->data() + m_packetRawData.startIndex + s_dataSizeV1Offset, sizeof(std::uint16_t));
			std::memcpy(&networkHeader.headerChecksum, m_packetRawData.buffer->data() + m_packetRawData.startIndex + s_headerChecksumV1Offset, sizeof(std::uint16_t));

			Test::Helpers::fromBigEndian(networkHeader.sourceAddress.v1);
			Test::Helpers::fromBigEndian(networkHeader.destinationAddress.v1);
			Test::Helpers::fromBigEndian(networkHeader.dataSize);
			Test::Helpers::fromBigEndian(networkHeader.headerChecksum);
		}
		else
		{
			std::memcpy(&networkHeader.sourceAddress.v2, m_packetRawData.buffer->data() + m_packetRawData.startIndex + s_sourceAddressOffset, sizeof(Test::NetworkV2AddressType));
			std::memcpy(&networkHeader.destinationAddress.v2, m_packetRawData.buffer->data() + m_packetRawData.startIndex + s_destinationAddressV2Offset, sizeof(Test::NetworkV2AddressType));
			std::memcpy(&networkHeader.protocol, m_packetRawData.buffer->data() + m_packetRawData.startIndex + s_protocolV2Offset, sizeof(std::uint8_t));
			std::memcpy(&networkHeader.dataSize, m_packetRawData.buffer->data() + m_packetRawData.startIndex + s_dataSizeV2Offset, sizeof(std::uint16_t));
			std::memcpy(&networkHeader.headerChecksum, m_packetRawData.buffer->data() + m_packetRawData.startIndex + s_headerChecksumV2Offset, sizeof(std::uint16_t));

			Test::Helpers::fromBigEndian(networkHeader.sourceAddress.v2);
			Test::Helpers::fromBigEndian(networkHeader.destinationAddress.v2);
			Test::Helpers::fromBigEndian(networkHeader.dataSize);
			Test::Helpers::fromBigEndian(networkHeader.headerChecksum);
		}

		std::vector<std::uint8_t> packetData;
		packetData.reserve(networkHeader.dataSize);

		auto backInserter = std::back_inserter(packetData);

		mutex.lock();

		std::system("cls");
		std::cout << ++counter << std::endl;

		mutex.unlock();

		std::copy(
			m_packetRawData.buffer->begin() + m_packetRawData.startIndex, 
			m_packetRawData.buffer->begin() + m_packetRawData.startIndex + networkHeader.dataSize,
			backInserter
		);

		return Test::NetworkPacket(Test::NetworkPacketData{ networkHeader, std::move(packetData) });
	}

private:
	PacketRawData m_packetRawData;
};

}

namespace Test
{

std::vector<NetworkPacket> PacketsReader::read(std::ifstream& stream)
{
	//ThreadPool pool;

	stream.seekg(0, std::ios::end);

	std::vector<std::uint8_t> buffer;
	buffer.reserve(stream.tellg());

	stream.seekg(0, std::ios::beg);

	while (!stream.eof())
	{
		buffer.push_back(stream.get());
	}

	std::vector<NetworkPacket> result;
	std::vector<std::future<NetworkPacket>> futures;

	for (std::size_t i = 0; i < buffer.size();)
	{
		assert(buffer[i] == 1 || buffer[i] == 2);

		const std::size_t dataSizeOffset = (buffer[i] == 1 ? s_dataSizeV1Offset : s_dataSizeV2Offset);
		const std::size_t dataOffset = (buffer[i] == 1 ? s_dataV1Offset : s_dataV2Offset);

		const std::uint16_t dataSize = ntohs(*reinterpret_cast<std::uint16_t*>(buffer.data() + i + dataSizeOffset));

		//futures.push_back(pool.pushTask(CreatePacketTask(PacketRawData{ &buffer, i })));
		futures.push_back(std::async(CreatePacketTask(PacketRawData{ &buffer, i })));

		i += dataOffset + dataSize;

		if (i == buffer.size() - 1)
		{
			break;
		}
	}

	result.reserve(futures.size());

	for (std::size_t i = 0; i < futures.size(); ++i)
	{
		result.push_back(futures[i].get());
	}

	return result;
}

}