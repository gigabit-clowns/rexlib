// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "deferred_release.hpp"
#include "memory_block_pool.hpp"
#include "queue_handle.hpp"

#include <xmipp4/core/span.hpp>

#include <cstddef>
#include <memory>
#include <mutex>

namespace xmipp4
{
namespace cuda
{

class event_recorder;
class memory_block;
class memory_source;

/**
 * @brief Hands out blocks of memory, keeping what it takes from the driver.
 *
 * Reaching the driver for every allocation is far too slow to do per
 * operation, so memory taken from it is kept and handed out again. What makes
 * that safe is knowing, for each block, what has to have finished before it
 * can be handed out once more.
 *
 * Blocks belong to the queue they were last handed to. That queue runs its
 * work in order, so giving one of its own blocks straight back to it costs
 * nothing. Taking one over from another queue costs a wait, which is scheduled
 * on the device rather than on the calling thread. Only when neither is
 * possible does the pool grow, doubling each time so that a workload settles
 * into asking the driver for nothing at all.
 *
 * A block whose owner is dropped while some other queue may still be using it
 * does not hold up that drop: it is held back until those queues have caught
 * up.
 *
 * Deals in blocks rather than in buffers, and knows nothing of the resource
 * they are said to come from. Turning one into the other is
 * @ref caching_memory_allocator's job.
 *
 * Thread safe.
 */
class memory_block_allocator
{
public:
	/**
	 * @brief Build an allocator over a source of memory.
	 *
	 * @param source Where the memory is taken from. Can not be nullptr.
	 * @param recorder Used to capture the points that say when a block can be
	 * handed out again. Can not be nullptr.
	 *
	 * @throws std::invalid_argument If @p source or @p recorder is nullptr.
	 */
	memory_block_allocator(
		std::unique_ptr<memory_source> source,
		std::unique_ptr<event_recorder> recorder
	);
	memory_block_allocator(const memory_block_allocator &other) = delete;
	memory_block_allocator(memory_block_allocator &&other) = delete;
	~memory_block_allocator();

	memory_block_allocator&
	operator=(const memory_block_allocator &other) = delete;
	memory_block_allocator&
	operator=(memory_block_allocator &&other) = delete;

	/**
	 * @brief Get the strictest alignment this allocator can satisfy.
	 *
	 * @return std::size_t Alignment, in bytes. A power of two.
	 */
	std::size_t get_max_alignment() const noexcept;

	/**
	 * @brief Check whether the host can address what this hands out.
	 *
	 * @return true The host can read and write it directly.
	 * @return false The memory has to be transferred to reach the host.
	 */
	bool is_host_accessible() const noexcept;

	/**
	 * @brief Hand out a block of memory.
	 *
	 * The block is rounded up to @ref get_max_alignment, so it may be larger
	 * than asked for.
	 *
	 * @param size Requested minimum size, in bytes.
	 * @param alignment Requested alignment, in bytes. Must be a power of two
	 * and not greater than @ref get_max_alignment.
	 * @param queue The queue the block is expected to be used on. May stand
	 * for no queue, in which case it can be used on any of them.
	 * @return memory_block& The block. Must be given back with @ref recycle.
	 *
	 * @throws std::bad_alloc If the device had no memory left.
	 * @throws std::invalid_argument If @p alignment is not a valid power of
	 * two within @ref get_max_alignment.
	 */
	memory_block& allocate(
		std::size_t size,
		std::size_t alignment,
		const queue_handle &queue
	);

	/**
	 * @brief Take a block back.
	 *
	 * @param block The block to take back, as handed out by @ref allocate.
	 * @param queues The queues besides its own that were given work
	 * referencing it. The block is held back until they have caught up.
	 */
	void recycle(
		memory_block &block,
		span<const queue_handle> queues
	) noexcept;

	/**
	 * @brief Give back to the driver whatever is not being used.
	 *
	 * Waits for every queue that has memory waiting on it, which is what makes
	 * the blocks scattered across them add back up to whole heaps.
	 *
	 * @return std::size_t Number of bytes given back.
	 *
	 * @throws error If a queue could not be waited for.
	 */
	std::size_t trim();

private:
	std::unique_ptr<memory_source> m_source;
	std::unique_ptr<event_recorder> m_recorder;

	std::mutex m_mutex;

	// Each of these gives what it holds back to the one before it, so they are
	// declared in the order that makes them die the other way around.
	memory_block_pool m_pool;
	deferred_release m_deferred;

	memory_block& acquire_block(
		std::size_t size,
		const queue_handle &queue
	);
	memory_block* take_over_foreign_block(
		std::size_t size,
		const queue_handle &queue
	);
	memory_block& grow(std::size_t size);
	std::size_t get_heap_size(std::size_t size) const noexcept;
	std::size_t trim_locked();
};

} // namespace cuda
} // namespace xmipp4
