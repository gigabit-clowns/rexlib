// SPDX-License-Identifier: GPL-3.0-only

#include "memory_block_allocator.hpp"

#include "memory_block.hpp"
#include "memory_heap.hpp"
#include "memory_source.hpp"

#include "../../config.hpp"
#include "../../logger.hpp"

#include <rexlib/core/binary/bit.hpp>
#include <rexlib/core/memory/align.hpp>

#include <algorithm>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rexlib
{
namespace cuda
{

memory_block_allocator::memory_block_allocator(
	std::unique_ptr<memory_source> source
)
	: m_source(std::move(source))
{
	if (!m_source)
	{
		throw std::invalid_argument(
			"memory_block_allocator::memory_block_allocator: A memory source is "
			"required."
		);
	}
}

memory_block_allocator::~memory_block_allocator()
{
	try
	{
		// The blocks that are held back have to go back while there is still
		// a pool to put them in, and every command has to be done with a
		// block before the heap under it is handed to the driver.
		trim_locked();
	}
	catch (...)
	{
		// Can not throw from a destructor. The blocks stay out of the pool,
		// which then reports them as never released.
		REXLIB_CUDA_LOG_ERROR(
			"An exception occurred waiting for the commands still using "
			"memory in ~memory_block_allocator()."
		);
	}
}

std::size_t memory_block_allocator::get_max_alignment() const noexcept
{
	return REXLIB_CUDA_CACHING_ALLOCATOR_MAX_ALIGNMENT_BYTES;
}

bool memory_block_allocator::is_host_accessible() const noexcept
{
	// The source is what took the memory, so it is what knows.
	return m_source->is_host_accessible();
}

memory_block& memory_block_allocator::allocate(
	std::size_t size,
	std::size_t alignment,
	const command_timeline *timeline
)
{
	if (!has_single_bit(alignment))
	{
		throw std::invalid_argument(
			"memory_block_allocator::allocate: The alignment must be a power "
			"of two."
		);
	}
	if (alignment > get_max_alignment())
	{
		throw std::invalid_argument(
			"memory_block_allocator::allocate: The alignment exceeds the "
			"maximum alignment of this allocator."
		);
	}

	// Rounding every request up to the strictest alignment the allocator
	// offers is what lets a block be cut in two without the remainder landing
	// somewhere it can no longer be handed out from. An empty block would have
	// no address of its own, so it gets the smallest there is.
	size = align_ceil(std::max<std::size_t>(size, 1), get_max_alignment());

	const std::lock_guard<std::mutex> lock(m_mutex);

	// Whatever has finished since the last allocation is worth knowing about
	// before deciding that nothing fits.
	m_deferred.process(m_pool);

	return acquire_block(size, timeline);
}

void memory_block_allocator::recycle(
	memory_block &block,
	span<const command_token> pending
) noexcept
{
	const std::lock_guard<std::mutex> lock(m_mutex);

	// A free block carries one token, so a block that a single command still
	// uses can go straight back.
	if (pending.size() < 2)
	{
		m_pool.release(
			block,
			pending.empty() ? command_token() : pending.front()
		);
		return;
	}

	try
	{
		m_deferred.defer(block, pending);
	}
	catch (...)
	{
		// Called from a destructor, so there is nowhere to report this to. The
		// block stays out of the pool, which is a leak, but handing out memory
		// a queue may still be reading from would be worse.
		REXLIB_CUDA_LOG_ERROR(
			"Could not hold a block back for the commands that use it. The "
			"block is lost."
		);
	}
}

std::size_t memory_block_allocator::trim()
{
	const std::lock_guard<std::mutex> lock(m_mutex);
	return trim_locked();
}

memory_block& memory_block_allocator::acquire_block(
	std::size_t size,
	const command_timeline *timeline
)
{
	auto *block = m_pool.find_suitable_block(size, timeline);

	if (!block)
	{
		// Its user has to wait for another queue, which the token it carries
		// lets it do. That is still cheaper than reaching the driver.
		block = m_pool.find_foreign_block(size, timeline);
	}

	if (!block)
	{
		try
		{
			block = &grow(size);
		}
		catch (const std::bad_alloc&)
		{
			REXLIB_CUDA_LOG_WARN(
				"Could not take more memory from the device. Retrying after "
				"giving back what is not being used."
			);

			// Everything the pool is holding on to but not using is memory the
			// driver could have handed over instead.
			trim_locked();

			// Trimming also merges back together what carrying tokens of
			// different timelines was keeping apart, so a request that did
			// not fit anywhere before may fit now without taking anything
			// more.
			block = m_pool.find_suitable_block(size, timeline);
			if (!block)
			{
				block = &grow(size);
			}
		}
	}

	// Handing out the whole block would leak whatever the request did not ask
	// for until the buffer dies.
	const auto remainder = block->get_size() - size;
	if (remainder > 0)
	{
		block = m_pool.partition_block(*block, size, remainder).first;
	}

	m_pool.acquire(*block);
	return *block;
}

memory_block& memory_block_allocator::grow(std::size_t size)
{
	auto heap_size = get_heap_size(size);

	// Asking for less is better than not getting anything, so a refusal is
	// answered by halving the request until it is only as big as it has to be.
	for (;;)
	{
		try
		{
			return *m_pool.register_heap(
				std::make_unique<memory_heap>(
					*m_source,
					heap_size,
					get_max_alignment()
				)
			);
		}
		catch (const std::bad_alloc&)
		{
			if (heap_size <= size)
			{
				throw;
			}

			// Kept aligned on the way down, so that cutting a block off a heap
			// can never leave the remainder somewhere a request with the
			// allocator's strictest alignment could not be served from.
			heap_size = std::max(
				size,
				align_ceil(heap_size / 2, get_max_alignment())
			);
		}
	}
}

std::size_t
memory_block_allocator::get_heap_size(std::size_t size) const noexcept
{
	// A request this big would spend most of a shared heap on itself, and is
	// worth giving back to the driver on its own once it is done with.
	if (size >= REXLIB_CUDA_CACHING_ALLOCATOR_LARGE_ALLOCATION_BYTES)
	{
		return size;
	}

	// Doubling means a workload that keeps growing stops reaching the driver
	// after a handful of allocations, rather than once per allocation.
	auto result = std::max<std::size_t>(m_pool.get_size(), size);
	result = std::max<std::size_t>(
		result,
		REXLIB_CUDA_CACHING_ALLOCATOR_MIN_HEAP_BYTES
	);
	result = std::min<std::size_t>(
		result,
		REXLIB_CUDA_CACHING_ALLOCATOR_MAX_HEAP_BYTES
	);
	result = align_ceil(result, get_max_alignment());

	// The clamp above is about how eagerly to grow, never about refusing a
	// request that is bigger than that.
	return std::max(result, size);
}

std::size_t memory_block_allocator::trim_locked()
{
	// Blocks that are held back are not free, so nothing can be merged
	// around them and no heap holding one can be given back.
	m_deferred.wait_all(m_pool);

	// The token of a free block only says what its next user has to wait
	// for. Once every one of those commands has finished there is nothing to
	// wait for, and blocks that were kept apart by the timelines of their
	// tokens add back up to whole heaps.
	std::vector<command_token> pending;
	m_pool.enumerate_pending(pending);
	for (const auto &token : pending)
	{
		token.wait();
	}

	m_pool.reset_pending();
	return m_pool.release_unused_heaps();
}

} // namespace cuda
} // namespace rexlib
