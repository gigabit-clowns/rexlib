// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/memory_allocator.hpp>

#include <cstddef>
#include <memory>

namespace rexlib
{

class command_timeline;

namespace cuda
{

class memory_block_allocator;

/**
 * @brief CUDA implementation of @ref rexlib::memory_allocator.
 *
 * Turns the blocks a @ref memory_block_allocator hands out into buffers, and
 * is what the rest of the framework sees. All the caching, and the knowledge
 * of what still uses a block that is handed out again, live in the block
 * allocator underneath.
 *
 * Buffers hold that block allocator rather than this, so they keep the memory
 * they were cut from alive whatever becomes of the allocator they came from.
 */
class caching_memory_allocator final
	: public memory_allocator
{
public:
	/**
	 * @brief Build an allocator over a source of blocks.
	 *
	 * @param resource The resource the memory belongs to. Must outlive the
	 * allocator.
	 * @param allocator Where the blocks come from. Can not be nullptr.
	 *
	 * @throws std::invalid_argument If @p allocator is nullptr.
	 */
	caching_memory_allocator(
		const memory_resource &resource,
		std::shared_ptr<memory_block_allocator> allocator
	);
	caching_memory_allocator(
		const caching_memory_allocator &other
	) = delete;
	caching_memory_allocator(caching_memory_allocator &&other) = delete;
	~caching_memory_allocator() override;

	caching_memory_allocator&
	operator=(const caching_memory_allocator &other) = delete;
	caching_memory_allocator&
	operator=(caching_memory_allocator &&other) = delete;

	const memory_resource& get_memory_resource() const noexcept override;

	std::size_t get_max_alignment() const noexcept override;

	std::shared_ptr<rexlib::buffer> allocate(
		std::size_t size,
		std::size_t alignment,
		rexlib::command_queue *queue_hint,
		command_token &pending
	) override;

	/**
	 * @brief Allocate a buffer for a timeline that is already resolved.
	 *
	 * What @ref allocate does once it has worked out which timeline the hint
	 * names. It has a name of its own because a null hint and a null
	 * timeline could not be told apart.
	 *
	 * @param size Requested minimum size, in bytes.
	 * @param alignment Requested alignment, in bytes. Must be a power of two
	 * and not greater than @ref get_max_alignment.
	 * @param timeline The timeline of the queue the buffer is expected to be
	 * used on first. May be nullptr.
	 * @param pending Receives the token of the command that still uses the
	 * memory handed out. Empty when nothing uses it.
	 * @return std::shared_ptr<rexlib::buffer> The buffer. Never null.
	 *
	 * @throws std::bad_alloc If the device had no memory left.
	 * @throws std::invalid_argument If @p alignment is not a valid power of
	 * two within @ref get_max_alignment.
	 */
	std::shared_ptr<rexlib::buffer> allocate_for_timeline(
		std::size_t size,
		std::size_t alignment,
		const command_timeline *timeline,
		command_token &pending
	);

	/**
	 * @brief Give back to the driver whatever is not being used.
	 *
	 * @return std::size_t Number of bytes given back.
	 *
	 * @throws std::exception If a command could not be waited for.
	 */
	std::size_t trim();

private:
	const memory_resource *m_resource;
	std::shared_ptr<memory_block_allocator> m_allocator;
};

} // namespace cuda
} // namespace rexlib
