// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{
namespace cuda
{

class queue_handle;

/**
 * @brief Captures points in a queue's timeline so that they can be waited on.
 *
 * The caching allocators need to know when work submitted before a given
 * moment has finished, so that the memory it referenced can be handed out
 * again. This is the only thing they need synchronization primitives for, and
 * it is deliberately the whole of what they can ask for.
 *
 * Points are handed out as tickets, which must be given back with
 * @ref release once the caller is done with them. @ref event_ticket does that
 * automatically and is what callers should hold.
 *
 * Implementations recycle the underlying primitives, so a released ticket may
 * be handed out again by a later @ref record.
 */
class event_recorder
{
public:
	/// Opaque handle to a captured point.
	using ticket = std::size_t;

	/// Value that no captured point is ever identified by.
	static constexpr ticket no_ticket = 0;

	event_recorder() noexcept = default;
	event_recorder(const event_recorder &other) = delete;
	event_recorder(event_recorder &&other) = delete;
	virtual ~event_recorder();

	event_recorder& operator=(const event_recorder &other) = delete;
	event_recorder& operator=(event_recorder &&other) = delete;

	/**
	 * @brief Capture the current point of a queue's timeline.
	 *
	 * @param queue The queue whose current point is captured.
	 * @return ticket Handle to the captured point. Never @ref no_ticket.
	 *
	 * @throws error If the point could not be captured.
	 * @pre @p queue must refer to a queue.
	 */
	virtual ticket record(const queue_handle &queue) = 0;

	/**
	 * @brief Give a ticket obtained from @ref record back.
	 *
	 * @param ticket The ticket to give back. @ref no_ticket is ignored.
	 */
	virtual void release(ticket ticket) noexcept = 0;

	/**
	 * @brief Check whether a captured point has been reached, without
	 * blocking.
	 *
	 * @param ticket The ticket to query.
	 * @return true The point has been reached.
	 * @return false The point has not been reached yet.
	 *
	 * @throws error If the point could not be queried.
	 */
	virtual bool is_complete(ticket ticket) = 0;

	/**
	 * @brief Block the calling thread until a captured point is reached.
	 *
	 * @param ticket The ticket to wait for.
	 *
	 * @throws error If the point could not be waited for.
	 */
	virtual void wait(ticket ticket) = 0;

	/**
	 * @brief Defer further work on a queue until a captured point is reached.
	 *
	 * Does not block the calling thread. The point may belong to a queue on
	 * another device.
	 *
	 * @param queue The queue whose further work is deferred.
	 * @param ticket The ticket to wait for.
	 *
	 * @throws error If the wait could not be scheduled.
	 * @pre @p queue must refer to a queue.
	 */
	virtual void enqueue_wait(const queue_handle &queue, ticket ticket) = 0;
};

} // namespace cuda
} // namespace rexlib
