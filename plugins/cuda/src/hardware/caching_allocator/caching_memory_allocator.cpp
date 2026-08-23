// SPDX-License-Identifier: GPL-3.0-only

#include "caching_memory_allocator.hpp"

#include "buffer.hpp"
#include "memory_block.hpp"
#include "memory_block_allocator.hpp"

#include "../command_queue.hpp"

#include <stdexcept>
#include <utility>

namespace xmipp4
{
namespace cuda
{

caching_memory_allocator::caching_memory_allocator(
	const memory_resource &resource,
	std::shared_ptr<memory_block_allocator> allocator
)
	: m_resource(&resource)
	, m_allocator(std::move(allocator))
{
	if (!m_allocator)
	{
		throw std::invalid_argument(
			"caching_memory_allocator::caching_memory_allocator: A block "
			"allocator is required."
		);
	}
}

caching_memory_allocator::~caching_memory_allocator() = default;

const memory_resource&
caching_memory_allocator::get_memory_resource() const noexcept
{
	return *m_resource;
}

std::size_t caching_memory_allocator::get_max_alignment() const noexcept
{
	return m_allocator->get_max_alignment();
}

std::shared_ptr<xmipp4::buffer> caching_memory_allocator::allocate(
	std::size_t size,
	std::size_t alignment,
	xmipp4::command_queue *queue_hint
)
{
	queue_handle queue;
	if (queue_hint)
	{
		queue = queue_handle(command_queue::cast(*queue_hint));
	}

	return allocate(
		size,
		alignment,
		queue
	);
}

std::shared_ptr<xmipp4::buffer> caching_memory_allocator::allocate(
	std::size_t size,
	std::size_t alignment,
	const queue_handle &queue
)
{
	auto &block = m_allocator->allocate(size, alignment, queue);
	try
	{
		// The host pointer is its own rather than the device one, since the
		// two only happen to coincide where the addresses are unified.
		return std::make_shared<buffer>(
			m_allocator,
			block,
			*m_resource,
			m_allocator->is_host_accessible() ? block.get_data() : nullptr
		);
	}
	catch (...)
	{
		// Nothing owns the block yet, so nothing would ever give it back.
		m_allocator->recycle(block, span<const queue_handle>());
		throw;
	}
}

std::size_t caching_memory_allocator::trim()
{
	return m_allocator->trim();
}

} // namespace cuda
} // namespace xmipp4
