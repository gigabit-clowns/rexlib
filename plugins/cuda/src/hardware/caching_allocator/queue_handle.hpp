// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cuda_runtime.h>

#include <cstdint>

namespace rexlib
{
namespace cuda
{

class command_queue;

/**
 * @brief The parts of a @ref command_queue that a memory pool needs.
 *
 * A pool outlives the queues that allocated from it, so it stores this by value
 * rather than keeping a pointer to a queue that may already be gone. The
 * ordinal travels along because an event can only be recorded on a stream of
 * the device it was created on, and there is no way to ask a stream which
 * device that is.
 *
 * A stream handle can be reused by the driver once its stream is destroyed, so
 * two queues may compare equal here at different points in time. That is
 * harmless: destroying a stream waits for the work submitted to it, so anything
 * this handle was used to order against has already completed by then.
 */
class queue_handle
{
public:
	/**
	 * @brief Construct a handle referring to no queue.
	 *
	 * Blocks bound to it belong to no timeline, which is what an allocation
	 * made without a queue hint gets.
	 */
	queue_handle() noexcept;

	/**
	 * @brief Construct a handle from its components.
	 *
	 * @param stream The stream backing the queue.
	 * @param ordinal Ordinal of the device the stream belongs to.
	 */
	queue_handle(cudaStream_t stream, int ordinal) noexcept;

	/**
	 * @brief Construct a handle referring to a queue.
	 *
	 * @param queue The queue to refer to.
	 */
	explicit queue_handle(const command_queue &queue) noexcept;

	queue_handle(const queue_handle &other) noexcept = default;
	queue_handle(queue_handle &&other) noexcept = default;
	~queue_handle() = default;

	queue_handle& operator=(const queue_handle &other) noexcept = default;
	queue_handle& operator=(queue_handle &&other) noexcept = default;

	/**
	 * @brief Get the stream backing the queue.
	 *
	 * @return cudaStream_t The stream, or null when this refers to no queue.
	 */
	cudaStream_t get_stream() const noexcept;

	/**
	 * @brief Get the device the stream belongs to.
	 *
	 * @return int The ordinal, or a negative value when this refers to no
	 * queue.
	 */
	int get_ordinal() const noexcept;

	/**
	 * @brief Check whether this refers to a queue at all.
	 *
	 * @return true It refers to a queue.
	 * @return false It refers to no queue.
	 */
	explicit operator bool() const noexcept;

	/// The stream is what identifies a queue; the ordinal only says where it
	/// lives, and two handles on one stream cannot disagree about it.
	friend bool operator==(
		const queue_handle &lhs,
		const queue_handle &rhs
	) noexcept
	{
		return lhs.get_stream() == rhs.get_stream();
	}

	friend bool operator!=(
		const queue_handle &lhs,
		const queue_handle &rhs
	) noexcept
	{
		return !(lhs == rhs);
	}

	/**
	 * @brief Total order over handles, so that they can key an ordered
	 * container.
	 *
	 * Built on the stream addresses rather than on the handles themselves,
	 * since comparing unrelated pointers with @c operator< is unspecified.
	 *
	 * @param lhs Left hand side of the comparison.
	 * @param rhs Right hand side of the comparison.
	 * @return true @p lhs orders before @p rhs.
	 * @return false @p lhs does not order before @p rhs.
	 */
	friend bool operator<(
		const queue_handle &lhs,
		const queue_handle &rhs
	) noexcept
	{
		return reinterpret_cast<std::uintptr_t>(lhs.get_stream()) <
		       reinterpret_cast<std::uintptr_t>(rhs.get_stream());
	}

private:
	cudaStream_t m_stream;
	int m_ordinal;
};

} // namespace cuda
} // namespace rexlib

#include "queue_handle.inl"
