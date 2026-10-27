// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_buffer.hpp"

#include "memory_block.hpp"
#include "memory_block_allocator.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace cuda
{

cuda_buffer::cuda_buffer(
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
			"cuda_buffer::cuda_buffer: An allocator is required to give the "
			"block back to."
		);
	}

	// A buffer that is never used still has to give the block back carrying
	// what it came with.
	if (!block.get_pending().is_empty())
	{
		m_uses.push_back(block.get_pending());
	}
}

cuda_buffer::~cuda_buffer()
{
	m_allocator->recycle(*m_block, get_recorded_uses());
}

void* cuda_buffer::get_host_ptr() noexcept
{
	return m_host_data;
}

const void* cuda_buffer::get_host_ptr() const noexcept
{
	return m_host_data;
}

std::size_t cuda_buffer::get_size() const noexcept
{
	return m_block->get_size();
}

const memory_resource& cuda_buffer::get_memory_resource() const noexcept
{
	return *m_resource;
}

void* cuda_buffer::get_device_ptr() noexcept
{
	return m_block->get_data();
}

const void* cuda_buffer::get_device_ptr() const noexcept
{
	return m_block->get_data();
}

void cuda_buffer::record_use(const command_token &token) const
{
	if (token.is_empty())
	{
		return;
	}

	const std::lock_guard<std::mutex> lock(m_mutex);

	const auto ite = std::find_if(
		m_uses.begin(), m_uses.end(),
		[&token] (const command_token &use) noexcept
		{
			return use.get_timeline() == token.get_timeline();
		}
	);

	if (ite == m_uses.end())
	{
		m_uses.push_back(token);
	}
	else if (token.get_id() > ite->get_id())
	{
		// Two threads may record on one timeline out of order.
		*ite = token;
	}
}

span<const command_token> cuda_buffer::get_recorded_uses() const noexcept
{
	return make_span(m_uses.data(), m_uses.size());
}

cuda_buffer& cuda_buffer::cast(buffer &buf)
{
	auto *result = dynamic_cast<cuda_buffer*>(&buf);
	if (!result)
	{
		throw std::invalid_argument(
			"The provided buffer was not created by the CUDA backend."
		);
	}

	return *result;
}

const cuda_buffer& cuda_buffer::cast(const buffer &buf)
{
	const auto *result = dynamic_cast<const cuda_buffer*>(&buf);
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
