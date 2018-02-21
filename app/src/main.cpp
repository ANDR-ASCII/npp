#include "data_structures.h"
#include "address_wrapper.h"
#include "packets_reader.h"
#include "helpers.h"

int main(int argc, char** argv)
{
	std::ifstream stream("network_0.raw", std::ios_base::binary);

	if (stream)
	{
		Test::PacketsReader::read(stream);
	}
	else
	{
		std::cout << "Cannot open the file";
	}

	std::cin.get();
	return 0;
}