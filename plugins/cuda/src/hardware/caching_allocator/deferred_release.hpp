// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "event_ticket.hpp"
#include "queue_handle.hpp"

#include "../../config.hpp"

#include <xmipp4/core/span.hpp>

#include <cstddef>
#include <vector>

#include <boost/container/small_vector.hpp>

namespace xmipp4
{
namespace cuda
{

class event_recorder;
class memory_block;
class memory_block_pool;

/**
 * @brief Holds back the blocks that work is still running against.
 *
 * A block goes back to its pool the moment nothing needs it any more, which
 * for a block used only on the queue it belongs to is as soon as its buffer
 * dies: that queue runs in order, so whatever it is handed to next is already
 * ordered after the work being dropped.
 *
 * A block that was also used on other queues has no such guarantee. This is
 * where those wait, one captured point per queue that touched them, until
 * every one of those points has been reached.
 *
 * Blocks wait here without being free, which is what keeps a neighbour coming
 * back to the pool from merging with a block that is still in use.
 */
class deferred_release
{
public:
	/**
	 * @brief Construct a deferred release holding nothing back.
	 */
	deferred_release() noexcept;
	deferred_release(const deferred_release &other) = delete;
	deferred_release(deferred_release &&other) = delete;
	~deferred_release();

	deferred_release& operator=(const deferred_release &other) = delete;
	deferred_release& operator=(deferred_release &&other) = delete;

	/**
	 * @brief Hold a block back until the queues that used it have caught up.
	 *
	 * @param recorder The recorder capturing the points to wait for. Must
	 * outlive this, since the points captured here are given back to it as
	 * they are reached.
	 * @param block The block to hold back. Must have been handed out, and must
	 * not already be held back.
	 * @param queues The queues that used the block besides the one it belongs
	 * to. Can not be empty, and none of them may stand for no queue.
	 *
	 * @throws std::invalid_argument If @p queues is empty or names no queue.
	 * @throws error If a point of one of the queues could not be captured. The
	 * block is then not held back, and is still not free.
	 */
	void defer(
		event_recorder &recorder,
		memory_block &block,
		span<const queue_handle> queues
	);

	/**
	 * @brief Give back every block whose queues have caught up.
	 *
	 * Does not block the calling thread.
	 *
	 * @param pool The pool the blocks came from.
	 *
	 * @throws error If a captured point could not be queried.
	 */
	void process(memory_block_pool &pool);

	/**
	 * @brief Give back every block, waiting for the queues that used them.
	 *
	 * @param pool The pool the blocks came from.
	 *
	 * @throws error If a captured point could not be waited for.
	 */
	void wait_all(memory_block_pool &pool);

	/**
	 * @brief Get how many blocks are currently held back.
	 *
	 * @return std::size_t Number of blocks.
	 */
	std::size_t get_pending_count() const noexcept;

private:
	/// One block and the points that have to be reached before it can go back.
	using pending_release = std::pair<memory_block*, std::vector<event_ticket>>;

	/// Whether each of one block's points has been reached. As long as how
	/// many queues touched a single buffer, which is a handful.
	using reached_point_flags = boost::container::small_vector<
		bool,
		XMIPP4_CUDA_CACHING_ALLOCATOR_SMALL_QUEUE_COUNT
	>;

	std::vector<pending_release> m_pending;

	/**
	 * @brief Give back the tickets of the points that have been reached.
	 *
	 * A block with no tickets left is one that nothing is running against any
	 * more.
	 *
	 * @param item The block to check the points of.
	 *
	 * @throws error If a captured point could not be queried. The block is
	 * then left waiting for exactly the points it was waiting for.
	 */
	static void drop_reached_points(pending_release &item);
};

} // namespace cuda
} // namespace xmipp4
