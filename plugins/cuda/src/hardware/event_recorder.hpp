// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

/**
 * @brief Captures points of streams so that they can be waited on.
 *
 * A point stands for the work issued on a stream before it was captured. It
 * is reached once all of that work has finished.
 *
 * Points are handed out as tickets, which must be given back with
 * @ref release once the caller is done with them. Implementations recycle the
 * underlying primitives, so a ticket that was given back may be handed out
 * again by a later @ref record.
 *
 * Thread safe. A point can be waited for while others are captured, queried
 * or given back.
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
	 * @brief Capture the current point of a stream.
	 *
	 * @param stream The stream whose current point is captured.
	 * @return ticket Handle to the captured point. Never @ref no_ticket.
	 *
	 * @throws error If the point could not be captured.
	 */
	virtual ticket record(cudaStream_t stream) = 0;

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
	 * @param ticket The ticket to wait for. It must not be given back while
	 * the call is waiting.
	 *
	 * @throws error If the point could not be waited for.
	 */
	virtual void wait(ticket ticket) = 0;

	/**
	 * @brief Defer further work on a stream until a captured point is
	 * reached.
	 *
	 * Does not block the calling thread. The point may belong to a stream of
	 * another device.
	 *
	 * @param stream The stream whose further work is deferred.
	 * @param ticket The ticket to wait for.
	 *
	 * @throws error If the wait could not be scheduled.
	 */
	virtual void enqueue_wait(cudaStream_t stream, ticket ticket) = 0;
};

} // namespace cuda
} // namespace rexlib
