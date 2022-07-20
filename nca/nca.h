#pragma once
#include <cstdint>
#include <vector>

class nca
{
public:
	static std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
	static std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);
};
