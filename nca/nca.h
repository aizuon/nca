#pragma once
#include <cstdint>
#include <vector>

class nca
{
public:
	static std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
	static std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);

private:
	static constexpr uint64_t symbol_size = sizeof(uint64_t);
	static constexpr uint64_t window_size = 2 * symbol_size;
};
