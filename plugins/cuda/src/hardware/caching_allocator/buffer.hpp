// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "queue_handle.hpp"

#include "../../config.hpp"

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/span.hpp>

#include <memory>

#include <boost/container/flat_set.hpp>
#include <boost/container/small_vector.hpp>

namespace rexlib
{

class command_queue;

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
 * that allocator, and keeps giving a block back off the public interface. It also keeps track of which queues besides the
 * one it was allocated for have been given work referencing it, since that is
 * what the allocator has to wait for before the block can be handed out again.
 *
 * The device pointer is not part of the @ref rexlib::buffer interface, so
 * reaching it means downcasting through @ref cast.
 *
 * @note Recording a use is not synchronized against other uses of the same
 * buffer, or against its destruction. A buffer submitted from several threads
 * at once needs the same external synchronization it already needs to be
 * shared at all.
 */
class buffer final
	: public rexlib::buffer
{
public:
	/**
	 * @brief Take ownership of a block.
	 *
	 * @param allocator The allocator the block belongs to. Held for as long as
	 * this lives, so that the block can always be given back. Can not be
	 * nullptr.
	 * @param block The block. Must have been handed out by @p allocator.
	 * @param resource The resource the memory is said to come from. Must
	 * outlive this.
	 * @param host_data Where the host can reach the block's memory, or nullptr
	 * if it cannot reach it at all. Kept as its own pointer rather than
	 * derived from the device one, since the two only happen to coincide where
	 * the addresses are unified.
	 */
	buffer(
		std::shared_ptr<memory_block_allocator> allocator,
		memory_block &block,
		const memory_resource &resource,
		void *host_data
	);
	buffer(const buffer &other) = delete;
	buffer(buffer &&other) = delete;
	~buffer() override;

	buffer& operator=(const buffer &other) = delete;
	buffer& operator=(buffer &&other) = delete;

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
	 * @brief Record that a queue has been given work referencing this buffer.
	 *
	 * Work on the queue the buffer was allocated for needs no recording: that
	 * queue runs in order, so anything it is given later is already ordered
	 * after it. Any other queue has to be waited for before the memory can be
	 * handed out again, and this is how the allocator finds out about it.
	 *
	 * @param queue The queue the work was given to.
	 *
	 * @throws std::invalid_argument If the queue was not created by the CUDA
	 * backend.
	 */
	void record_use(const rexlib::command_queue &queue);

	/**
	 * @brief Record that a queue that is already resolved has been given work
	 * referencing this buffer.
	 *
	 * What @ref record_use does once it has worked out which queue it was
	 * handed. Callers inside this backend usually hold a CUDA queue already,
	 * and this spares them the downcast.
	 *
	 * @param queue The queue the work was given to.
	 */
	void record_use(const queue_handle &queue);

	/**
	 * @brief Get the queues that were recorded as using this buffer.
	 *
	 * The queue the buffer was allocated for is never among them.
	 *
	 * @return span<const queue_handle> The queues.
	 */
	span<const queue_handle> get_recorded_queues() const noexcept;

	/**
	 * @brief Downcast a buffer to this backend's implementation.
	 *
	 * @param buf The buffer to be cast.
	 * @return buffer& The same buffer, as a CUDA buffer.
	 *
	 * @throws std::invalid_argument If the buffer was not created by this
	 * backend.
	 */
	static buffer& cast(rexlib::buffer &buf);

	/**
	 * @brief Downcast a buffer to this backend's implementation.
	 *
	 * @param buf The buffer to be cast.
	 * @return const buffer& The same buffer, as a CUDA buffer.
	 *
	 * @throws std::invalid_argument If the buffer was not created by this
	 * backend.
	 */
	static const buffer& cast(const rexlib::buffer &buf);

private:
	/// Sorted and deduplicated, and inline for as many queues as a buffer is
	/// ever likely to be handed to.
	using queue_set_type = boost::container::flat_set<
		queue_handle,
		std::less<queue_handle>,
		boost::container::small_vector<
			queue_handle,
			REXLIB_CUDA_CACHING_ALLOCATOR_SMALL_QUEUE_COUNT
		>
	>;

	std::shared_ptr<memory_block_allocator> m_allocator;
	memory_block *m_block;
	const memory_resource *m_resource;
	void *m_host_data;
	queue_set_type m_queues;
};

} // namespace cuda
} // namespace rexlib
