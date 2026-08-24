// SPDX-License-Identifier: GPL-3.0-only

#include "host_memory_allocator.hpp"

#include <rex/core/hardware/buffer.hpp>
#include <rex/core/hardware/command_queue.hpp>
#include <rex/core/memory/align.hpp>
#include <rex/core/system/host.hpp>
#include <rex/core/platform/assert.hpp>

#include "host_memory_resource.hpp"
#include "host_buffer.hpp"

#include <stdexcept>

namespace rexlib
{

std::shared_ptr<host_memory_allocator> host_memory_allocator::m_instance;

const memory_resource& 
host_memory_allocator::get_memory_resource() const noexcept
{
	return host_memory_resource::get();
}

std::size_t host_memory_allocator::get_max_alignment() const noexcept
{
    return get_page_size();
}

std::shared_ptr<buffer> host_memory_allocator::allocate(
	std::size_t size, 
	std::size_t alignment, 
	command_queue* /*queue_hint*/
)
{
	size = align_ceil(size, alignment);
	return std::make_shared<host_buffer>(size, alignment);
}

host_memory_allocator& host_memory_allocator::get()
{
	return *(create());
}

const std::shared_ptr<host_memory_allocator>& host_memory_allocator::create()
{
	if (!m_instance)
	{
		m_instance = std::make_shared<host_memory_allocator>();
	}

	REXLIB_ASSERT(m_instance);
	return m_instance;
}

} // namespace rexlib
