// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../../config.hpp"

#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <utility>
#include <vector>

#include <boost/container/small_vector.hpp>

namespace rexlib
{
namespace cuda
{

class memory_block;
class memory_block_pool;

/**
 * @brief Holds back the blocks that several commands are still running
 * against.
 *
 * A free block carries one token, which is what its next user waits for. A
 * block whose buffer dies while commands of several queues still use it
 * would need one token per queue. This is where such a block waits, until at
 * most one of those commands is still running. It then goes back to its pool
 * carrying that token, or an empty one.
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
	 * @brief Hold a block back until at most one of the commands that use it
	 * is still running.
	 *
	 * @param block The block to hold back. Must have been handed out, and must
	 * not already be held back.
	 * @param pending The tokens of the commands that still use the block.
	 * There must be at least two.
	 *
	 * @throws std::invalid_argument If @p pending holds fewer than two tokens.
	 * The block is then not held back, and is still not free.
	 */
	void defer(memory_block &block, span<const command_token> pending);

	/**
	 * @brief Give back every block that at most one command still uses.
	 *
	 * Does not block the calling thread.
	 *
	 * @param pool The pool the blocks came from.
	 *
	 * @throws std::exception If a token could not be queried.
	 */
	void process(memory_block_pool &pool);

	/**
	 * @brief Give back every block, waiting for the commands that use them.
	 *
	 * @param pool The pool the blocks came from.
	 *
	 * @throws std::exception If a token could not be waited for.
	 */
	void wait_all(memory_block_pool &pool);

	/**
	 * @brief Get how many blocks are currently held back.
	 *
	 * @return std::size_t Number of blocks.
	 */
	std::size_t get_pending_count() const noexcept;

private:
	/// One block and the tokens of the commands that still use it.
	using pending_release =
		std::pair<memory_block*, std::vector<command_token>>;

	/// Whether each of one block's commands has finished. As long as how many
	/// queues touched a single buffer, which is a handful.
	using completion_flags = boost::container::small_vector<
		bool,
		REXLIB_CUDA_CACHING_ALLOCATOR_SMALL_QUEUE_COUNT
	>;

	std::vector<pending_release> m_pending;

	/**
	 * @brief Forget the tokens of the commands that have finished.
	 *
	 * @param item The block to check the tokens of.
	 *
	 * @throws std::exception If a token could not be queried. The block is
	 * then left waiting for exactly the tokens it was waiting for.
	 */
	static void drop_complete_tokens(pending_release &item);
};

} // namespace cuda
} // namespace rexlib
