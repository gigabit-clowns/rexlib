// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "event_recorder.hpp"

namespace rexlib
{
namespace cuda
{

class queue_handle;

/**
 * @brief Owning handle to a point captured by an @ref event_recorder.
 *
 * Gives the ticket back to its recorder on destruction, so that a throw on the
 * way out of an allocation cannot strand the underlying primitive.
 */
class event_ticket
{
public:
	/**
	 * @brief Construct a handle owning nothing.
	 *
	 * Behaves as an already reached point: waiting on it returns immediately
	 * and querying it reports completion.
	 */
	event_ticket() noexcept;

	/**
	 * @brief Capture the current point of a queue's timeline.
	 *
	 * @param recorder The recorder capturing the point. Must outlive this.
	 * @param queue The queue whose current point is captured.
	 *
	 * @throws error If the point could not be captured.
	 * @pre @p queue must refer to a queue.
	 */
	event_ticket(event_recorder &recorder, const queue_handle &queue);

	event_ticket(const event_ticket &other) = delete;
	event_ticket(event_ticket &&other) noexcept;
	~event_ticket();

	event_ticket& operator=(const event_ticket &other) = delete;
	event_ticket& operator=(event_ticket &&other) noexcept;

	/**
	 * @brief Exchange the contents of two handles.
	 *
	 * @param other The handle to exchange contents with.
	 */
	void swap(event_ticket &other) noexcept;

	/**
	 * @brief Give the ticket back to its recorder, leaving this owning
	 * nothing.
	 */
	void reset() noexcept;

	/**
	 * @brief Check whether this owns a captured point.
	 *
	 * @return true It owns a captured point.
	 * @return false It owns nothing.
	 */
	explicit operator bool() const noexcept;

	/**
	 * @brief Check whether the captured point has been reached, without
	 * blocking.
	 *
	 * @return true The point has been reached, or this owns nothing.
	 * @return false The point has not been reached yet.
	 *
	 * @throws error If the point could not be queried.
	 */
	bool is_complete() const;

	/**
	 * @brief Block the calling thread until the captured point is reached.
	 *
	 * Returns immediately if this owns nothing.
	 *
	 * @throws error If the point could not be waited for.
	 */
	void wait() const;

	/**
	 * @brief Defer further work on a queue until the captured point is
	 * reached.
	 *
	 * Does nothing if this owns nothing.
	 *
	 * @param queue The queue whose further work is deferred.
	 *
	 * @throws error If the wait could not be scheduled.
	 * @pre @p queue must refer to a queue.
	 */
	void enqueue_wait(const queue_handle &queue) const;

private:
	event_recorder *m_recorder;
	event_recorder::ticket m_ticket;
};

} // namespace cuda
} // namespace rexlib
