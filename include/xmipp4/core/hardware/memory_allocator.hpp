// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>

#include <xmipp4/core/platform/dynamic_shared_object.h>

namespace xmipp4
{

class buffer;
class command_queue;
class memory_resource;

/**
 * @brief Abstract allocator of @ref buffer objects on a @ref memory_resource.
 *
 * Concrete implementations are free to back themselves with anything that fits 
 * the underlying backend.
 *
 * The allocator is also in charge of tracking the asynchronous use of the 
 * buffers it produces: when a buffer's shared ownership is dropped while device 
 * work referencing it is still in flight, the allocator defers the actual 
 * release until that work has completed.
 */
class REXLIB_API memory_allocator
{
public:
	memory_allocator() noexcept;
	memory_allocator(const memory_allocator &other) = delete;
	memory_allocator(memory_allocator &&other) = delete;
	virtual ~memory_allocator();

	memory_allocator& operator=(const memory_allocator &other) = delete;
	memory_allocator& operator=(memory_allocator &&other) = delete;

	/**
	 * @brief Get the memory resource backing this allocator.
	 *
	 * @return Reference to the backing memory resource.
	 */
	virtual
	const memory_resource& get_memory_resource() const noexcept = 0;

	/**
	 * @brief Get the largest alignment this allocator can satisfy.
	 *
	 * The returned value is the strictest alignment that may be passed to 
	 * @ref allocate. It is a power of two and is constant over the lifetime of
	 * the allocator.
	 *
	 * @return Maximum supported alignment, in bytes.
	 */
	virtual std::size_t get_max_alignment() const noexcept = 0;

	/**
	 * @brief Allocate a new buffer on the backing memory resource.
	 *
	 * The allocator may round @p size up to satisfy internal alignment or page
	 * granularity constraints; the resulting buffer's @ref buffer::get_size 
	 * may return a number greater than or equal to @p size .
	 *
	 * @param size Requested minimum size, in bytes. 
	 * @param alignment Requested alignment, in bytes. Must be a power
	 * of two and not greater than @ref get_max_alignment.
	 * @param queue_hint Optional non-owning pointer to the queue on
	 * which the buffer is expected to be first used. Some backends may defer 
	 * the allocation until the current execution point is reached on this
	 * queue. Thus, providing a queue_hint and then using the buffer on another
	 * queue leads to undefined behavior.
	 * @return The newly allocated buffer. Never null.
	 *
	 * @throws std::bad_alloc (or a backend-specific exception derived
	 * from @c std::exception) on allocation failure.
	 * @throws std::invalid_argument if @p alignment is not a valid
	 * power of two within @ref get_max_alignment.
	 */
	virtual
	std::shared_ptr<buffer> allocate(
		std::size_t size,
		std::size_t alignment,
		command_queue *queue_hint = nullptr
	) = 0;
};

} // namespace xmipp4
