// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/system/memory_mapped_file.hpp>

#include "memory_mapped_file_handle.hpp"

namespace rexlib
{

memory_mapped_file::memory_mapped_file() noexcept
	: m_size(0UL)
	, m_data(nullptr)
{
}

memory_mapped_file::memory_mapped_file(
	const char* filename, 
	access_flags access,
	std::size_t size,
	bool copy_on_write
)
	: m_size(size)
	, m_data(memory_mapped_file_open(filename, access, m_size, copy_on_write))
{
}

memory_mapped_file::memory_mapped_file(
	const std::string& filename, 
	access_flags access,
	std::size_t size,
	bool copy_on_write
)
	: memory_mapped_file(filename.c_str(), access, size, copy_on_write)
{
}

memory_mapped_file::memory_mapped_file(memory_mapped_file&& other) noexcept
	: m_size(other.m_size)
	, m_data(other.m_data)
{
	other.m_size = 0UL;
	other.m_data = nullptr;
}

memory_mapped_file::~memory_mapped_file()
{
	close();
}

memory_mapped_file& memory_mapped_file::operator=(memory_mapped_file&& other) noexcept
{
	close();
	swap(other);
	return *this;
}

bool memory_mapped_file::is_open() const noexcept
{
	return m_data != nullptr;
}

void memory_mapped_file::swap(memory_mapped_file& other) noexcept
{
	std::swap(m_size, other.m_size);
	std::swap(m_data, other.m_data);
}

void memory_mapped_file::open(
	const char* filename, 
	access_flags access,
	std::size_t size,
	bool copy_on_write
)
{
	*this = memory_mapped_file(filename, access, size, copy_on_write);
}

void memory_mapped_file::open(
	const std::string& filename, 
	access_flags access,
	std::size_t size,
	bool copy_on_write
)
{
	open(filename.c_str(), access, size, copy_on_write);
}

void memory_mapped_file::close() noexcept
{
	if (m_data != nullptr)
	{
		memory_mapped_file_close(m_data, m_size);
		m_size = 0UL;
		m_data = nullptr;
	}
}

std::size_t memory_mapped_file::size() const noexcept
{
	return m_size;
}

void* memory_mapped_file::data() const noexcept
{
	return m_data;
}

void swap(memory_mapped_file& lhs, memory_mapped_file& rhs) noexcept
{
	lhs.swap(rhs);
}

} // namespace rexlib
