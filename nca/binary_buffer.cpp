#include "pch.h"
#include "binary_buffer.h"

binary_buffer::binary_buffer(const std::vector<uint8_t>& obj)
	: buffer_(obj), write_offset_(obj.size())
{
}

binary_buffer::binary_buffer(std::vector<uint8_t>&& obj)
	: buffer_(std::move(obj)), write_offset_(obj.size())
{
}

binary_buffer::binary_buffer(const binary_buffer& obj)
	: buffer_(obj.buffer_), write_offset_(obj.write_offset_), read_offset_(obj.read_offset_)
{
}

binary_buffer::binary_buffer(binary_buffer&& obj) noexcept
	: buffer_(std::move(obj.buffer_)), write_offset_(obj.write_offset_), read_offset_(obj.read_offset_)
{
}

binary_buffer& binary_buffer::operator=(const binary_buffer& obj)
{
	buffer_ = obj.buffer_;
	write_offset_ = obj.write_offset_;
	read_offset_ = obj.read_offset_;

	return *this;
}

binary_buffer& binary_buffer::operator=(binary_buffer&& obj) noexcept
{
	std::swap(buffer_, obj.buffer_);
	std::swap(write_offset_, obj.write_offset_);
	std::swap(read_offset_, obj.read_offset_);

	return *this;
}

void binary_buffer::write_size(uint64_t obj)
{
	write(obj);
}

void binary_buffer::write(const std::string& obj)
{
	std::scoped_lock lock(mutex_);

	const uint64_t size = obj.size();
	write_size(size);

	const uint64_t length = size * sizeof(std::string::value_type);
	grow_if_needed(length);
	for (const auto o : obj)
	{
		write(o);
	}
}

void binary_buffer::write_raw(const std::string& obj)
{
	std::scoped_lock lock(mutex_);

	const uint64_t length = obj.size();
	grow_if_needed(length);
	for (const auto o : obj)
	{
		write(o);
	}
}

bool binary_buffer::read_size(uint64_t& obj)
{
	return read(obj);
}

bool binary_buffer::read(std::string& obj)
{
	std::scoped_lock lock(mutex_);

	uint64_t size = 0;
	if (!read_size(size))
		return false;

	const uint64_t length = size * sizeof(std::string::value_type);

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

bool binary_buffer::operator==(const binary_buffer& obj) const
{
	if (write_offset_ != obj.write_offset_)
		return false;

	if (read_offset_ != obj.read_offset_)
		return false;

	return buffer_ == obj.buffer_;
}

void binary_buffer::grow_if_needed(uint64_t write_length)
{
	const uint64_t final_length = write_offset_ + write_length;
	const bool reserve_needed = buffer_.capacity() <= final_length;
	const bool resize_needed = buffer_.size() <= final_length;

	if (reserve_needed)
		buffer_.reserve(final_length * buffer_grow_factor);

	if (resize_needed)
		buffer_.resize(final_length);
}
