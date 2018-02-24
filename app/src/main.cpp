#include "packets_statistic_collector.h"

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		std::cout << "this_program_name.exe [file_to_parse]" << std::endl;

		std::cin.get();
		return 0;
	}

	std::string filepath = argv[1];

	Test::PacketsStatisticCollector statCollector(filepath);

	std::cout << "All network packets count: " << statCollector.networkPacketsCount() << std::endl;
	std::cout << "Network V1 packets count: " << statCollector.networkV1PacketsCount() << std::endl;
	std::cout << "Network V2 packets count: " << statCollector.networkV2PacketsCount() << std::endl;
	std::cout << "Network V1 unique addresses count: " << statCollector.networkV1UniqueAddrsCount() << std::endl;
	std::cout << "Network V2 unique addresses count: " << statCollector.networkV2UniqueAddrsCount() << std::endl;
	std::cout << "Transport V1 packets count: " << statCollector.transportV1PacketsCount() << std::endl;
	std::cout << "Transport V2 packets count: " << statCollector.transportV2PacketsCount() << std::endl;
	std::cout << "Transport V1 packets with broken checksum count: " << statCollector.transportV1PacketsBrokenChecksumCount() << std::endl;
	std::cout << "Transport V2 packets with broken checksum count: " << statCollector.transportV2PacketsBrokenChecksumCount() << std::endl;

	std::cin.get();
	return 0;
}