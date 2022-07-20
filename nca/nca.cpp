#include "pch.h"
#include "nca.h"

#include <cmath>
#include <span>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <ranges>

std::vector<uint8_t> nca::compress(const std::vector<uint8_t>& data)
{
	static constexpr size_t window_size = 16;

	if (data.size() < std::pow(window_size, 3))
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

		auto duplicates = std::views::filter(windows, [&window, i](const auto& p) -> bool
		{
			if (p.first == i)
			{
				return false;
			}

			for (size_t j = 0; j < window_size; j++)
			{
				if (p.second[j] != window[j])
				{
					return false;
				}
			}

			return true;
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
					const auto key_it = std::ranges::find_if(duplicate_windows, [&window](const auto& p) -> bool
					{
						for (size_t i = 0; i < window_size; i++)
						{
							if (p.first[i] != window[i])
							{
								return false;
							}
						}

						return true;
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

	return {};
}

std::vector<uint8_t> nca::decompress(const std::vector<uint8_t>& data)
{
	return {};
}
