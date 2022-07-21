#include "pch.h"
#include "utils.h"

#include <chrono>
#include <ranges>
#include <boost/algorithm/hex.hpp>

const bool utils::is_little_endian = is_little_endian_cast();

bool utils::is_little_endian_cast()
{
	const uint32_t i = 1;

	return reinterpret_cast<const uint8_t*>(&i)[0] == i;
}
