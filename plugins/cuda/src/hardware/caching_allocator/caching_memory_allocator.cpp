// SPDX-License-Identifier: GPL-3.0-only

#include "caching_memory_allocator.hpp"

#include "cuda_buffer.hpp"
#include "memory_block.hpp"
#include "memory_block_allocator.hpp"

#include "../cuda_command_queue.hpp"
#include "../stream_timeline.hpp"

#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/span.hpp>

#include <stdexcept>
#include <utility>

namespace rexlib
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

std::shared_ptr<buffer> caching_memory_allocator::allocate(
	std::size_t size,
	std::size_t alignment,
	command_queue *queue_hint,
	command_token &pending
)
{
	// A queue of another backend has no memory here that it could use at no
	// cost, so it is as good as no hint.
	const auto *queue = dynamic_cast<const cuda_command_queue*>(queue_hint);
	return allocate_for_timeline(
		size,
		alignment,
		queue ? queue->get_timeline().get() : nullptr,
		pending
	);
}

std::shared_ptr<buffer>
caching_memory_allocator::allocate_for_timeline(
	std::size_t size,
	std::size_t alignment,
	const command_timeline *timeline,
	command_token &pending
)
{
	auto &block = m_allocator->allocate(size, alignment, timeline);
	try
	{
		// A command that has finished leaves nothing to wait for. Asked
		// before the buffer exists, so that failing to ask leaves the block
		// with a single owner to give it back.
		if (block.get_pending().is_complete())
		{
			block.set_pending(command_token());
		}

		// The host pointer is its own rather than the device one, since the
		// two only happen to coincide where the addresses are unified.
		auto result = std::make_shared<cuda_buffer>(
			m_allocator,
			block,
			*m_resource,
			m_allocator->is_host_accessible() ? block.get_data() : nullptr
		);

		pending = block.get_pending();
		return result;
	}
	catch (...)
	{
		// Nothing owns the block yet, so nothing would ever give it back. It
		// goes back carrying what it came with.
		const auto token = block.get_pending();
		m_allocator->recycle(block, make_span(&token, 1));
		throw;
	}
}

std::size_t caching_memory_allocator::trim()
{
	return m_allocator->trim();
}

} // namespace cuda
} // namespace rexlib
