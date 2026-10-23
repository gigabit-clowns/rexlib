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

class command_timeline;

namespace cuda
{

class memory_heap;

/**
 * @brief The cache of a caching allocator: heaps, and the blocks they are cut
 * into.
 *
 * Free blocks are kept sorted by the timeline of the token they carry first
 * and by their size second, which makes finding the best fit for a queue a
 * lower bound. Every block, free or not, is also kept in address order, which
 * makes coalescing a look at the neighbours.
 *
 * Blocks carrying an empty token are the cheapest to hand out, since nothing
 * can still be running against them. A heap starts out that way.
 *
 * Two free neighbours whose tokens are of one timeline become one block that
 * carries the token with the greater id. The pool therefore requires that a
 * timeline finishes its commands in the order of their ids, which is what a
 * stream does.
 *
 * The pool never talks to a device and never waits. Waiting for the token of
 * a block it hands out is the caller's to arrange.
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
	 * @brief Find the best free block a queue can use at no cost.
	 *
	 * Considers the blocks whose token is of @p timeline, which a queue
	 * waits for by running in order, and the blocks whose token is empty,
	 * which nothing is running against at all.
	 *
	 * @param size Minimum size of the block, in bytes.
	 * @param timeline The timeline of the queue that would use the block.
	 * May be nullptr, in which case only blocks with an empty token are
	 * considered.
	 * @return memory_block* The smallest suitable block, or nullptr if there
	 * is none.
	 */
	memory_block* find_suitable_block(
		std::size_t size,
		const command_timeline *timeline
	) noexcept;

	/**
	 * @brief Find the best free block whose token is of some other timeline.
	 *
	 * @param size Minimum size of the block, in bytes.
	 * @param timeline The timeline whose blocks are left out. May be
	 * nullptr.
	 * @return memory_block* The smallest suitable block carrying a token of
	 * another timeline, or nullptr if there is none.
	 */
	memory_block* find_foreign_block(
		std::size_t size,
		const command_timeline *timeline
	) noexcept;

	/**
	 * @brief Hand a block out.
	 *
	 * The block keeps the token it carries, which is what its next user has
	 * to wait for.
	 *
	 * @param block The block to hand out. Must be free.
	 */
	void acquire(memory_block &block) noexcept;

	/**
	 * @brief Take a block back, merging it with its free neighbours.
	 *
	 * @param block The block to take back. Must have been handed out.
	 * @param pending Token of the last command that used the block. Empty if
	 * nothing uses it.
	 */
	void release(memory_block &block, command_token pending) noexcept;

	/**
	 * @brief Cut a free block in two.
	 *
	 * Both halves are left free and carrying the token of the block they
	 * came from.
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
	 * The heap arrives as one free block carrying an empty token, since
	 * nothing has ever run against it.
	 *
	 * @param heap The heap to take in. Can not be nullptr.
	 * @return memory_block* The block spanning the whole heap.
	 *
	 * @throws std::bad_alloc If the heap could not be tracked.
	 */
	memory_block* register_heap(std::unique_ptr<memory_heap> heap);

	/**
	 * @brief List what the free blocks are still used by.
	 *
	 * Lists one token per timeline: the one with the greatest id among the
	 * free blocks of that timeline. Empty tokens are not listed.
	 *
	 * @param pending Overwritten with the tokens.
	 */
	void enumerate_pending(std::vector<command_token> &pending) const;

	/**
	 * @brief Empty the token of every free block, merging what that allows.
	 *
	 * Turns blocks kept apart by the timelines of their tokens back into
	 * whatever contiguous runs they add up to, which is what makes a heap
	 * releasable again after the pool has been used from several queues.
	 *
	 * @pre Every command reported by @ref enumerate_pending must have
	 * finished, otherwise a block may be handed out while it is still in use.
	 */
	void reset_pending() noexcept;

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
	 * @brief Orders free blocks by the timeline of their token first and by
	 * size second.
	 *
	 * Grouping by timeline is what turns "the best fit this queue can have
	 * for free" into a single lower bound.
	 */
	class free_memory_block_compare
	{
	public:
		/// What a free block is sorted by, for searching without a block.
		using key_type = std::pair<const command_timeline*, std::size_t>;

		bool operator()(
			const memory_block &lhs,
			const memory_block &rhs
		) const noexcept;
		bool operator()(
			const memory_block &lhs,
			const key_type &rhs
		) const noexcept;
		bool operator()(
			const key_type &lhs,
			const memory_block &rhs
		) const noexcept;

	private:
		static key_type get_key(const memory_block &block) noexcept;
		static bool
		is_less(const key_type &lhs, const key_type &rhs) noexcept;
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

	memory_block* find_in_timeline(
		std::size_t size,
		const command_timeline *timeline
	) noexcept;
	free_memory_block_set_type::const_iterator
	find_next_timeline(const command_timeline *timeline) const noexcept;

	void consider_merging_block(memory_block &block) noexcept;
	void consider_merging_forwards(memory_block &block) noexcept;
	void consider_merging_backwards(memory_block &block) noexcept;
	bool is_mergeable(
		const memory_block &block,
		const memory_block &neighbour
	) const noexcept;
	static void
	keep_later_pending(memory_block &block, const memory_block &other) noexcept;
	bool is_partition(const memory_block &block) const noexcept;
};

} // namespace cuda
} // namespace rexlib
