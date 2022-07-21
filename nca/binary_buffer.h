#pragma once
#include <cassert>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include <boost/endian/conversion.hpp>

#include "utils.h"

class binary_buffer
{
public:
	binary_buffer() = default;
	binary_buffer(const std::vector<uint8_t>& obj);
	binary_buffer(std::vector<uint8_t>&& obj);

	binary_buffer(const binary_buffer& obj);
	binary_buffer(binary_buffer&& obj) noexcept;
	binary_buffer& operator=(const binary_buffer& obj);
	binary_buffer& operator=(binary_buffer&& obj) noexcept;

	~binary_buffer() = default;

	inline const std::vector<uint8_t>& buffer() const
	{
		return buffer_;
	}

	inline std::vector<uint8_t>& writable_buffer()
	{
		return buffer_;
	}

	inline uint64_t size() const
	{
		return buffer_.size();
	}

	inline uint64_t write_offset() const
	{
		return write_offset_;
	}

	inline uint64_t read_offset() const
	{
		return read_offset_;
	}

	inline void grow(uint64_t size)
	{
		assert(size < buffer_.size());

		buffer_.resize(size);
	}

	inline void reserve(uint64_t size)
	{
		buffer_.reserve(size);
	}

	void write_size(uint64_t obj);

	template <typename T>
	void write(T obj)
	{
		static_assert(std::is_arithmetic_v<T>);

		std::scoped_lock lock(mutex_);

		if (!utils::is_little_endian)
		{
			boost::endian::endian_reverse_inplace(obj);
		}

		const uint64_t length = sizeof(T);
		grow_if_needed(length);
		memcpy(buffer_.data() + write_offset_, &obj, length);
		write_offset_ += length;
	}

	template <typename T>
	void write(const std::vector<T>& obj)
	{
		std::scoped_lock lock(mutex_);

		const uint64_t size = obj.size();
		write_size(size);

		const uint64_t length = size * sizeof(T);
		grow_if_needed(length);
		for (auto o : obj)
		{
			write(o);
		}
	}

	template <typename T>
	void write_raw(const std::vector<T>& obj)
	{
		std::scoped_lock lock(mutex_);

		const uint64_t length = obj.size() * sizeof(T);
		grow_if_needed(length);
		for (auto o : obj)
		{
			write(o);
		}
	}

	void write(const std::string& obj);

	void write_raw(const std::string& obj);

	bool read_size(uint64_t& obj);

	template <typename T>
	bool read(T& obj)
	{
		static_assert(std::is_arithmetic_v<T>);

		std::scoped_lock lock(mutex_);

		const uint64_t length = sizeof(T);

		const uint64_t final_offset = read_offset_ + length;
		if (buffer_.size() < final_offset)
			return false;

		memcpy(&obj, buffer_.data() + read_offset_, length);
		if (!utils::is_little_endian)
		{
			boost::endian::endian_reverse_inplace(obj);
		}
		read_offset_ = final_offset;

		return true;
	}

	template <typename T>
	bool read(std::vector<T>& obj)
	{
		std::scoped_lock lock(mutex_);

		uint64_t size = 0;
		if (!read_size(size))
			return false;

		const uint64_t length = size * sizeof(T);

		const uint64_t final_offset = read_offset_ + length;
		if (buffer_.size() < final_offset)
			return false;

		obj.resize(size);
		for (uint64_t i = 0; i < size; i++)
		{
			if (!read(obj[i]))
				return false;
		}

		return true;
	}

	bool read(std::string& obj);

	bool operator==(const binary_buffer& obj) const;

private:
	std::vector<uint8_t> buffer_;
	uint64_t write_offset_ = 0;
	uint64_t read_offset_ = 0;

	std::recursive_mutex mutex_;

	static constexpr float buffer_grow_factor = 1.5f;

	void grow_if_needed(uint64_t write_length);
};
