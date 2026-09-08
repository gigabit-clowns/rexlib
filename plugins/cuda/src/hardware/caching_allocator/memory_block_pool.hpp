// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "memory_block.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include <boost/intrusive/list.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/unordered/unordered_flat_map.hpp>

namespace rexlib
{
namespace cuda
{

class memory_heap;

/**
 * @brief The cache of a caching allocator: heaps, and the blocks they are cut
 * into.
 *
 * Free blocks are kept sorted by the queue they belong to first and by their
 * size second, which makes finding the best fit for a queue a lower bound.
 * Every block, free or not, is also kept in address order, which makes
 * coalescing a look at the neighbours.
 *
 * Blocks belonging to no queue are the cheapest to hand out, since nothing can
 * still be running against them. A heap starts out that way and only becomes
 * bound to a queue once it has been handed to one.
 *
 * The pool never talks to a device. Whatever the caller has to wait for before
 * taking a block over from another queue is the caller's to arrange.
 */
class memory_block_pool
{
public:
	memory_block_pool() noexcept;
	memory_block_pool(const memory_block_pool &other) = delete;
	memory_block_pool(memory_block_pool &&other) = delete;
	~memory_block_pool();

	memory_block_pool& operator=(const memory_block_pool &other) = delete;
	memory_block_pool& operator=(memory_block_pool &&other) = delete;

	/**
	 * @brief Find the best free block a queue can take without waiting.
	 *
	 * Considers the blocks belonging to @p queue, whose work is ordered
	 * against anything the queue submits next, and the blocks belonging to no
	 * queue, which nothing is running against at all.
	 *
	 * @param size Minimum size of the block, in bytes.
	 * @param queue The queue that would use the block.
	 * @return memory_block* The smallest suitable block, or nullptr if there
	 * is none.
	 */
	memory_block* find_suitable_block(
		std::size_t size,
		const queue_handle &queue
	) noexcept;

	/**
	 * @brief Find the best free block belonging to some other queue.
	 *
	 * The caller must defer its own work until the returned block's queue has
	 * caught up before handing the block out, since work submitted to that
	 * queue may still be running against it.
	 *
	 * @param size Minimum size of the block, in bytes.
	 * @param queue The queue that would take the block over.
	 * @return memory_block* The smallest suitable block belonging to another
	 * queue, or nullptr if there is none.
	 */
	memory_block* find_foreign_block(
		std::size_t size,
		const queue_handle &queue
	) noexcept;

	/**
	 * @brief Hand a block out, binding it to a queue.
	 *
	 * @param block The block to hand out. Must be free.
	 * @param queue The queue the block is handed to.
	 */
	void acquire(memory_block &block, const queue_handle &queue) noexcept;

	/**
	 * @brief Take a block back, merging it with its free neighbours.
	 *
	 * @param block The block to take back. Must have been handed out.
	 */
	void release(memory_block &block) noexcept;

	/**
	 * @brief Cut a free block in two.
	 *
	 * Both halves are left free and belonging to the same queue as the block
	 * they came from.
	 *
	 * @param block The block to cut. Must be free.
	 * @param first_size Size of the first half, in bytes.
	 * @param second_size Size of the second half, in bytes. Together with
	 * @p first_size it must add up to the size of @p block.
	 * @return std::pair<memory_block*, memory_block*> The two halves, in
	 * address order.
	 *
	 * @throws std::bad_alloc If the second block could not be tracked.
	 */
	std::pair<memory_block*, memory_block*> partition_block(
		memory_block &block,
		std::size_t first_size,
		std::size_t second_size
	);

	/**
	 * @brief Take a new heap into the pool.
	 *
	 * The heap arrives as one free block belonging to no queue, since nothing
	 * has ever run against it.
	 *
	 * @param heap The heap to take in. Can not be nullptr.
	 * @return memory_block* The block spanning the whole heap.
	 *
	 * @throws std::bad_alloc If the heap could not be tracked.
	 */
	memory_block* register_heap(std::unique_ptr<memory_heap> heap);

	/**
	 * @brief List the queues that free blocks currently belong to.
	 *
	 * The queue standing for no queue is not listed, since there is nothing to
	 * wait for on it.
	 *
	 * @param queues Overwritten with the queues.
	 */
	void enumerate_queues(std::vector<queue_handle> &queues) const;

	/**
	 * @brief Unbind every free block from its queue, merging what that allows.
	 *
	 * Turns blocks scattered across per-queue sets back into whatever
	 * contiguous runs they add up to, which is what makes a heap releasable
	 * again after the pool has been used from several queues.
	 *
	 * @pre The work of every queue reported by @ref enumerate_queues must have
	 * finished, otherwise a block may be handed out while it is still in use.
	 */
	void reset_queues() noexcept;

	/**
	 * @brief Give back every heap that is not cut up or in use.
	 *
	 * @return std::size_t Number of bytes given back.
	 */
	std::size_t release_unused_heaps() noexcept;

	/**
	 * @brief Get how much memory the pool holds.
	 *
	 * @return std::size_t Total size of the heaps, in bytes, whether their
	 * blocks are handed out or not.
	 */
	std::size_t get_size() const noexcept;

	/**
	 * @brief Get how many blocks are currently handed out.
	 *
	 * @return std::size_t Number of blocks.
	 */
	std::size_t get_acquired_block_count() const noexcept;

private:
	/**
	 * @brief Orders free blocks by queue first and by size second.
	 *
	 * Grouping by queue is what turns "the best fit this queue can have for
	 * free" into a single lower bound.
	 */
	class free_memory_block_compare
	{
	public:
		bool operator()(
			const memory_block &lhs,
			const memory_block &rhs
		) const noexcept;
	};

	using memory_block_list_type = boost::intrusive::list<
		memory_block,
		boost::intrusive::member_hook<
			memory_block,
			memory_block::block_list_hook_type,
			&memory_block::block_list_hook
		>
	>;
	using free_memory_block_set_type = boost::intrusive::multiset<
		memory_block,
		boost::intrusive::member_hook<
			memory_block,
			memory_block::free_block_set_hook_type,
			&memory_block::free_block_set_hook
		>,
		boost::intrusive::compare<free_memory_block_compare>
	>;
	using heap_map_type = boost::unordered::unordered_flat_map<
		const memory_heap*,
		std::unique_ptr<memory_heap>
	>;

	memory_block_list_type m_blocks;
	free_memory_block_set_type m_free_blocks;
	heap_map_type m_heaps;
	std::size_t m_size;

	memory_block* find_in_queue(
		std::size_t size,
		const queue_handle &queue
	) noexcept;

	void consider_merging_block(memory_block &block) noexcept;
	void consider_merging_forwards(memory_block &block) noexcept;
	void consider_merging_backwards(memory_block &block) noexcept;
	bool is_mergeable(
		const memory_block &block,
		const memory_block &neighbour
	) const noexcept;
	bool is_partition(const memory_block &block) const noexcept;
};

} // namespace cuda
} // namespace rexlib
