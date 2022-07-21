#include "pch.h"
#include "nca.h"

#include <cstring>
#include <span>
#include <utility>
#include <unordered_map>
#include <map>
#include <ranges>

std::vector<uint8_t> nca::compress(const std::vector<uint8_t>& data)
{
	std::vector compressed_data(data);

	const auto compare_windows = [](const std::span<const uint8_t>& a, const std::span<const uint8_t>& b) -> bool
	{
		return std::memcmp(a.data(), b.data(), window_size) == 0;
	};

	if (data.size() < window_size * window_size * window_size)
		throw std::exception("input data is too small to be compressed");

	std::unordered_map<uint64_t, std::span<const uint8_t>> windows;
	std::map<uint64_t, const std::span<const uint8_t>&> duplicate_windows;

	for (uint64_t i = 0; i < data.size() - window_size + 1; i++)
	{
		windows.emplace(i, std::span(data.data(), window_size));
	}

	for (uint64_t i = 0; i < windows.size(); i++)
	{
		const auto& window = windows[i];

		auto duplicates = std::views::filter(windows, [&compare_windows, &window, i](const auto& p) -> bool
		{
			if (p.first == i)
			{
				return false;
			}

			return compare_windows(p.second, window);
		});

		bool add = false;
		for (const auto duplicate : duplicates | std::views::keys)
		{
			const uint64_t dist = i > duplicate ? i - duplicate : duplicate - i;
			if (dist >= window_size)
			{
				duplicate_windows.insert({ duplicate, window });
				add = true;
			}
		}

		if (add)
		{
			duplicate_windows.insert({ i, window });
			i += window_size;
		}
	}

	std::unordered_map<uint64_t, std::pair<const std::span<const uint8_t>&, std::set<uint64_t>>> compression_map;
	uint64_t last_symbol = 0;
	const auto next_symbol = [&last_symbol, &windows]() -> uint64_t
	{
		while (last_symbol < std::numeric_limits<uint64_t>::max())
		{
			last_symbol++;
			const auto window_it = std::ranges::find_if(windows, [&last_symbol](const auto& p) -> bool
			{
				for (uint64_t i = 0; i < window_size - sizeof(last_symbol) + 1; i++)
				{
					if (std::memcmp(&p.second[i], &last_symbol, sizeof(last_symbol)) == 0)
					{
						return true;
					}
				}

				return false;
			});
			if (window_it == windows.end())
			{
				return last_symbol;
			}
		}

		throw std::exception("no available symbol value exists");
	};

	for (const auto& duplicate : duplicate_windows | std::views::reverse)
	{
		auto compression_it = std::ranges::find_if(compression_map,
		                                           [&compare_windows, &duplicate](const auto& p) -> bool
		                                           {
			                                           return compare_windows(p.second.first, duplicate.second);
		                                           });
		uint64_t current_symbol = 0;
		if (compression_it != compression_map.end())
		{
			current_symbol = compression_it->first;
		}
		else
		{
			current_symbol = next_symbol();
			compression_it = compression_map.insert({ current_symbol, { duplicate.second, {} } }).first;
		}
		compressed_data.erase(compressed_data.begin() + duplicate.first + window_size - symbol_size,
		                      compressed_data.begin() + duplicate.first + window_size);
		std::memcpy(&compressed_data[duplicate.first], &current_symbol, sizeof(current_symbol));
		compression_it->second.second.insert(duplicate.first);
	}

	//TODO: construct compression header

	return compressed_data;
}

std::vector<uint8_t> nca::decompress(const std::vector<uint8_t>& data)
{
	std::vector decompressed_data(data);

	return decompressed_data;
}
