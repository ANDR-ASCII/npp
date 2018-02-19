#include "data_structures.h"
#include "address_wrapper.h"

int main(int argc, char** argv)
{
	Npp::AddressWrapper addr(0x350101ff);

	std::cout << addr.toString();

	std::cin.get();
	return 0;
}