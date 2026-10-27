// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../../config.hpp"

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/span.hpp>

#include <memory>
#include <mutex>

#include <boost/container/small_vector.hpp>

namespace rexlib
{
namespace cuda
{

class memory_block;
class memory_block_allocator;

/**
 * @brief CUDA implementation of @ref rexlib::buffer.
 *
 * Owns a block of a @ref memory_block_allocator for as long as it lives, and
 * gives it back when it dies. Holding the block allocator rather than the
 * @ref caching_memory_allocator it came from is what lets the memory outlive
 * that allocator, and keeps giving a block back off the public interface.
 *
 * It also remembers the commands that use its memory: the last one of each
 * timeline, starting with the one its block came with. A buffer may die while
 * they are still running, and they are what the next user of the block has to
 * wait for.
 *
 * The device pointer is not part of the @ref rexlib::buffer interface, so
 * reaching it means downcasting through @ref cast.
 *
 * @note Recording a use is synchronized against other uses of the same
 * buffer, but not against its destruction.
 */
class cuda_buffer final
	: public buffer
{
public:
	/**
	 * @brief Take ownership of a block.
	 *
	 * @param allocator The allocator the block belongs to. Held for as long as
	 * this lives, so that the block can always be given back. Can not be
	 * nullptr.
	 * @param block The block. Must have been handed out by @p allocator. A
	 * non empty token on it is recorded as a use.
	 * @param resource The resource the memory is said to come from. Must
	 * outlive this.
	 * @param host_data Where the host can reach the block's memory, or nullptr
	 * if it cannot reach it at all. Kept as its own pointer rather than
	 * derived from the device one, since the two only happen to coincide where
	 * the addresses are unified.
	 */
	cuda_buffer(
		std::shared_ptr<memory_block_allocator> allocator,
		memory_block &block,
		const memory_resource &resource,
		void *host_data
	);
	cuda_buffer(const cuda_buffer &other) = delete;
	cuda_buffer(cuda_buffer &&other) = delete;
	~cuda_buffer() override;

	cuda_buffer& operator=(const cuda_buffer &other) = delete;
	cuda_buffer& operator=(cuda_buffer &&other) = delete;

	void* get_host_ptr() noexcept override;
	const void* get_host_ptr() const noexcept override;
	std::size_t get_size() const noexcept override;
	const memory_resource& get_memory_resource() const noexcept override;

	/**
	 * @brief Get a device accessible pointer to the data.
	 *
	 * @return void* Pointer to the start of the buffer's data. Never null.
	 */
	void* get_device_ptr() noexcept;

	/**
	 * @brief Get a device accessible pointer to the data.
	 *
	 * @return const void* Pointer to the start of the buffer's data. Never
	 * null.
	 */
	const void* get_device_ptr() const noexcept;

	/**
	 * @brief Record that a command uses this buffer.
	 *
	 * Only the last command of each timeline is kept, so the timeline of
	 * @p token must finish its commands in the order of their ids. An empty
	 * token is ignored.
	 *
	 * A command that only reads the buffer uses it as well, so this can be
	 * called on a buffer that cannot be written.
	 *
	 * Thread safe.
	 *
	 * @param token The token of the command.
	 */
	void record_use(const command_token &token) const;

	/**
	 * @brief Get the commands that were recorded as using this buffer.
	 *
	 * Not synchronized against @ref record_use.
	 *
	 * @return span<const command_token> One token per timeline.
	 */
	span<const command_token> get_recorded_uses() const noexcept;

	/**
	 * @brief Downcast a buffer to this backend's implementation.
	 *
	 * @param buf The buffer to be cast.
	 * @return cuda_buffer& The same buffer, as a CUDA buffer.
	 *
	 * @throws std::invalid_argument If the buffer was not created by this
	 * backend.
	 */
	static cuda_buffer& cast(buffer &buf);

	/**
	 * @brief Downcast a buffer to this backend's implementation.
	 *
	 * @param buf The buffer to be cast.
	 * @return const cuda_buffer& The same buffer, as a CUDA buffer.
	 *
	 * @throws std::invalid_argument If the buffer was not created by this
	 * backend.
	 */
	static const cuda_buffer& cast(const buffer &buf);

private:
	/// One token per timeline, inline for as many queues as a buffer is ever
	/// likely to be handed to.
	using token_vector_type = boost::container::small_vector<
		command_token,
		REXLIB_CUDA_CACHING_ALLOCATOR_SMALL_QUEUE_COUNT
	>;

	std::shared_ptr<memory_block_allocator> m_allocator;
	memory_block *m_block;
	const memory_resource *m_resource;
	void *m_host_data;
	mutable std::mutex m_mutex;
	mutable token_vector_type m_uses;
};

} // namespace cuda
} // namespace rexlib
