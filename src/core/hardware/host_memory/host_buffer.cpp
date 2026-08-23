// SPDX-License-Identifier: GPL-3.0-only

#include "host_buffer.hpp"

#include <rex/core/memory/aligned_alloc.hpp>
#include <rex/core/memory/align.hpp>

#include "host_memory_resource.hpp"

#include <stdexcept>

namespace rex
{

host_buffer::host_buffer(
	std::size_t size, 
	std::size_t alignment
)
	: m_data(nullptr)
	, m_size(0UL)
{
	m_size = align_ceil(size, alignment);
	m_data = aligned_alloc(m_size, alignment);

	if (m_size > 0 && m_data == nullptr)
	{
		throw std::bad_alloc();
	}
}

host_buffer::~host_buffer()
{
	aligned_free(get_host_ptr());
}


void* host_buffer::get_host_ptr() noexcept
{
	return m_data;
}

const void* host_buffer::get_host_ptr() const noexcept
{
	return m_data;
}

std::size_t host_buffer::get_size() const noexcept
{
	return m_size;
}

const memory_resource& host_buffer::get_memory_resource() const noexcept
{
	return host_memory_resource::get();
}

} // namespace rex
