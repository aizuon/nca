#include "pch.h"
#include "nca.h"

#include <cmath>
#include <cstring>
#include <span>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <ranges>

std::vector<uint8_t> nca::compress(const std::vector<uint8_t>& data)
{
	std::vector compressed_data(data);

	const auto compare_windows = [](const std::span<uint8_t>& a, const std::span<uint8_t>& b) -> bool
	{
		return std::memcmp(a.data(), b.data(), window_size) == 0;
	};

	if (data.size() < window_size * window_size * window_size)
		throw std::exception("input data is too small to be compressed");

	std::unordered_map<size_t, std::span<uint8_t>> windows;
	std::vector<std::pair<const std::span<uint8_t>&, std::unordered_set<size_t>>> duplicate_windows;

	for (size_t i = 0; i < data.size() - window_size + 1; i++)
	{
		windows[i] = std::span(const_cast<uint8_t*>(&data[i]), window_size);
	}

	for (size_t i = 0; i < windows.size(); i++)
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

		std::unordered_set<size_t>* keys = nullptr;
		bool first = true;
		bool add = false;
		for (const auto duplicate : duplicates | std::views::keys)
		{
			const size_t dist = i > duplicate ? i - duplicate : duplicate - i;
			if (dist >= window_size)
			{
				if (first)
				{
					const auto key_it = std::ranges::find_if(duplicate_windows,
					                                         [&compare_windows, &window](const auto& p) -> bool
					                                         {
						                                         return compare_windows(p.first, window);
					                                         });

					keys = key_it != duplicate_windows.end()
						       ? &key_it->second
						       : &duplicate_windows.emplace_back(window, std::initializer_list<size_t>{}).second;

					first = false;
				}
				keys->insert(duplicate);
				add = true;
			}
		}

		if (add)
		{
			keys->insert(i);
			i += window_size;
		}
	}

	std::unordered_map<uint64_t, std::pair<const std::span<uint8_t>&, std::unordered_set<size_t>>> compression_map;
	uint64_t last_symbol = 0;
	const auto next_symbol = [&last_symbol, &windows]() -> uint64_t
	{
		uint64_t symbol = last_symbol + 1;
		while (symbol < std::numeric_limits<uint64_t>::max())
		{
			const auto window_it = std::ranges::find_if(windows, [symbol](const auto& p) -> bool
			{
				for (size_t i = 0; i < window_size - sizeof(symbol) + 1; i++)
				{
					if (std::memcmp(&p.second[i], &symbol, sizeof(symbol)) == 0)
					{
						return true;
					}
				}

				return false;
			});
			if (window_it == windows.end())
				return symbol;

			symbol++;
		}

		throw std::exception("no available symbol value exists");
	};

	return compressed_data;
}

std::vector<uint8_t> nca::decompress(const std::vector<uint8_t>& data)
{
	std::vector decompressed_data(data);

	return decompressed_data;
}
