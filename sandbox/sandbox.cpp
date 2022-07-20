#include <cstdint>
#include <exception>
#include <fstream>
#include <vector>

#include "../nca/nca.h"

int main()
{
	std::ifstream text_file("sample_data\\lorem_ipsum.txt", std::ios::binary);
	if (!text_file.good())
		throw std::exception("text file could not be opened");
	std::vector<uint8_t> text_data(std::istreambuf_iterator(text_file), {});

	const auto text_data_compressed = nca::compress(text_data);
}
