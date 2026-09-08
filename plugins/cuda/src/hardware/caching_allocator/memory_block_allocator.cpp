// SPDX-License-Identifier: GPL-3.0-only

#include "memory_block_allocator.hpp"

#include "event_recorder.hpp"
#include "event_ticket.hpp"
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
	std::unique_ptr<memory_source> source,
	std::unique_ptr<event_recorder> recorder
)
	: m_source(std::move(source))
	, m_recorder(std::move(recorder))
{
	if (!m_source)
	{
		throw std::invalid_argument(
			"memory_block_allocator::memory_block_allocator: A memory source is "
			"required."
		);
	}

	if (!m_recorder)
	{
		throw std::invalid_argument(
			"memory_block_allocator::memory_block_allocator: An event recorder "
			"is required."
		);
	}
}

memory_block_allocator::~memory_block_allocator()
{
	try
	{
		// The blocks still waiting on a queue have to go back while there is
		// still a pool to put them in, and the queues have to be done with them
		// before the heaps under them are handed to the driver.
		m_deferred.wait_all(m_pool);
	}
	catch (...)
	{
		// Can not throw from a destructor. The blocks stay out of the pool,
		// which then reports them as never released.
		REXLIB_CUDA_LOG_ERROR(
			"An exception occurred waiting for the queues still using memory "
			"in ~memory_block_allocator()."
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
	const queue_handle &queue
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

	return acquire_block(size, queue);
}

void memory_block_allocator::recycle(
	memory_block &block,
	span<const queue_handle> queues
) noexcept
{
	const std::lock_guard<std::mutex> lock(m_mutex);

	if (queues.empty())
	{
		m_pool.release(block);
		return;
	}

	try
	{
		m_deferred.defer(*m_recorder, block, queues);
	}
	catch (...)
	{
		// Called from a destructor, so there is nowhere to report this to. The
		// block stays out of the pool, which is a leak, but handing out memory
		// a queue may still be reading from would be worse.
		REXLIB_CUDA_LOG_ERROR(
			"Could not hold a block back for the queues that used it. The "
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
	const queue_handle &queue
)
{
	auto *block = m_pool.find_suitable_block(size, queue);

	if (!block)
	{
		block = take_over_foreign_block(size, queue);
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

			// Trimming also merges back together what belonging to different
			// queues was keeping apart, so a request that did not fit
			// anywhere before may fit now without taking anything more.
			block = m_pool.find_suitable_block(size, queue);
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

	m_pool.acquire(*block, queue);
	return *block;
}

memory_block* memory_block_allocator::take_over_foreign_block(
	std::size_t size,
	const queue_handle &queue
)
{
	// A block of another queue can only be taken over by deferring this
	// queue's work until that one has caught up, and an allocation made
	// without naming a queue has nowhere to put that wait.
	if (!queue)
	{
		return nullptr;
	}

	auto *block = m_pool.find_foreign_block(size, queue);
	if (!block)
	{
		return nullptr;
	}

	// Captured now rather than when the block was given back: the point a
	// queue is at now is necessarily past the work that was using this block,
	// so it says the same thing and costs one capture per takeover instead of
	// one per release.
	const event_ticket ticket(*m_recorder, block->get_queue());
	ticket.enqueue_wait(queue);

	return block;
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
	// Blocks still waiting on a queue are not free, so nothing can be merged
	// around them and no heap holding one can be given back.
	m_deferred.wait_all(m_pool);

	// What each free block belongs to only says when it can be handed out.
	// Once every one of those queues is idle the answer is "now" for all of
	// them, and blocks that were kept apart by belonging to different queues
	// add back up to whole heaps.
	std::vector<queue_handle> queues;
	m_pool.enumerate_queues(queues);
	for (const auto &queue : queues)
	{
		const event_ticket ticket(*m_recorder, queue);
		ticket.wait();
	}

	m_pool.reset_queues();
	return m_pool.release_unused_heaps();
}

} // namespace cuda
} // namespace rexlib
