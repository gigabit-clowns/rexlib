// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "deferred_release.hpp"
#include "memory_block_pool.hpp"

#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <memory>
#include <mutex>

namespace rexlib
{

class command_timeline;

namespace cuda
{

class memory_block;
class memory_source;

/**
 * @brief Hands out blocks of memory, keeping what it takes from the driver.
 *
 * Reaching the driver for every allocation is far too slow to do per
 * operation, so memory taken from it is kept and handed out again. What makes
 * that safe is knowing, for each block, what has to have finished before it
 * can be used once more.
 *
 * A block comes back with the tokens of the commands that still use it, and
 * is handed out again carrying one. Whoever uses it next waits for that
 * token; this never waits on anyone's behalf, and schedules no wait.
 *
 * A queue waits for its own commands by running in order, so a block whose
 * token is of that queue's timeline costs it nothing, and is what it is given
 * first. A block of another timeline comes next. Only when there is neither
 * does the pool grow, doubling each time so that a workload settles into
 * asking the driver for nothing at all.
 *
 * A block that commands of several queues still use is held back until at
 * most one of them is still running, since it can carry a single token.
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
	 *
	 * @throws std::invalid_argument If @p source is nullptr.
	 */
	explicit memory_block_allocator(std::unique_ptr<memory_source> source);
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
	 * @param timeline The timeline of the queue the block is expected to be
	 * used on first, whose blocks are preferred. May be nullptr.
	 * @return memory_block& The block. Its token says what its user has to
	 * wait for. Must be given back with @ref recycle.
	 *
	 * @throws std::bad_alloc If the device had no memory left.
	 * @throws std::invalid_argument If @p alignment is not a valid power of
	 * two within @ref get_max_alignment.
	 */
	memory_block& allocate(
		std::size_t size,
		std::size_t alignment,
		const command_timeline *timeline
	);

	/**
	 * @brief Take a block back.
	 *
	 * @param block The block to take back, as handed out by @ref allocate.
	 * @param pending The tokens of the commands that still use the block.
	 * With more than one, the block is held back until at most one of them is
	 * still running.
	 */
	void recycle(
		memory_block &block,
		span<const command_token> pending
	) noexcept;

	/**
	 * @brief Give back to the driver whatever is not being used.
	 *
	 * Waits for every command that still uses a free block, which is what
	 * makes the blocks kept apart by their tokens add back up to whole heaps.
	 *
	 * @return std::size_t Number of bytes given back.
	 *
	 * @throws std::exception If a command could not be waited for.
	 */
	std::size_t trim();

private:
	std::unique_ptr<memory_source> m_source;

	std::mutex m_mutex;

	// Each of these gives what it holds back to the one before it, so they are
	// declared in the order that makes them die the other way around.
	memory_block_pool m_pool;
	deferred_release m_deferred;

	memory_block& acquire_block(
		std::size_t size,
		const command_timeline *timeline
	);
	memory_block& grow(std::size_t size);
	std::size_t get_heap_size(std::size_t size) const noexcept;
	std::size_t trim_locked();
};

} // namespace cuda
} // namespace rexlib
