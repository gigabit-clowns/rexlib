// SPDX-License-Identifier: GPL-3.0-only

#include "buffer.hpp"

#include "memory_block.hpp"
#include "memory_block_allocator.hpp"

#include "../command_queue.hpp"

#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace cuda
{

buffer::buffer(
	std::shared_ptr<memory_block_allocator> allocator,
	memory_block &block,
	const memory_resource &resource,
	void *host_data
)
	: m_allocator(std::move(allocator))
	, m_block(&block)
	, m_resource(&resource)
	, m_host_data(host_data)
{
	if (!m_allocator)
	{
		throw std::invalid_argument(
			"buffer::buffer: An allocator is required to give the block back "
			"to."
		);
	}
}

buffer::~buffer()
{
	m_allocator->recycle(*m_block, get_recorded_queues());
}

void* buffer::get_host_ptr() noexcept
{
	return m_host_data;
}

const void* buffer::get_host_ptr() const noexcept
{
	return m_host_data;
}

std::size_t buffer::get_size() const noexcept
{
	return m_block->get_size();
}

const memory_resource& buffer::get_memory_resource() const noexcept
{
	return *m_resource;
}

void* buffer::get_device_ptr() noexcept
{
	return m_block->get_data();
}

const void* buffer::get_device_ptr() const noexcept
{
	return m_block->get_data();
}

void buffer::record_use(const rexlib::command_queue &queue)
{
	record_use(queue_handle(command_queue::cast(queue)));
}

void buffer::record_use(const queue_handle &queue)
{
	// The queue the block belongs to runs its work in order, so what it is
	// given after this is already ordered after it. Recording it would only
	// make the allocator wait for something it does not have to.
	if (queue != m_block->get_queue())
	{
		m_queues.insert(queue);
	}
}

span<const queue_handle> buffer::get_recorded_queues() const noexcept
{
	if (m_queues.empty())
	{
		return span<const queue_handle>();
	}

	return make_span(&(*m_queues.begin()), m_queues.size());
}

buffer& buffer::cast(rexlib::buffer &buf)
{
	auto *result = dynamic_cast<buffer*>(&buf);
	if (!result)
	{
		throw std::invalid_argument(
			"The provided buffer was not created by the CUDA backend."
		);
	}

	return *result;
}

const buffer& buffer::cast(const rexlib::buffer &buf)
{
	const auto *result = dynamic_cast<const buffer*>(&buf);
	if (!result)
	{
		throw std::invalid_argument(
			"The provided buffer was not created by the CUDA backend."
		);
	}

	return *result;
}

} // namespace cuda
} // namespace rexlib
