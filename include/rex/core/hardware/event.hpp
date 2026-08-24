// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/platform/dynamic_shared_object.h>
#include <rex/core/hardware/event_usage_flags.hpp>

namespace rexlib
{

/**
 * @brief Abstract synchronization primitive between queues and the host.
 *
 * An @c event marks a point in a command queue's execution timeline that
 * can later be waited on or queried, either by another queue (possibly on a
 * different device) or by the host thread.
 *
 * Signaling and queue-side waiting are performed by the command queue itself,
 * via @ref command_queue::signal and @ref command_queue::wait. A newly
 * constructed event is in the signaled state. Any wait or query issued before
 * the first @ref command_queue::signal on this event completes immediately
 * and reports the event as signaled.
 *
 * @ref command_queue::signal may be invoked multiple times over the lifetime
 * of the event. Each call defines a new target point in the corresponding
 * queue's timeline and supersedes any previous one; subsequent waits and
 * queries refer to the most recently recorded point. There is no need for the
 * user to reset the event between uses; backends that require an explicit
 * reset handle it internally. The event can only be signaled from the device
 * used to create it, even if @ref event_usage_flag_bits::cross_device_wait is
 * set.
 *
 * The supported set of operations is determined at creation time by the
 * @ref event_usage_flags passed to the factory, and can be queried
 * via @ref get_supported_usage. Backends are free to pick the cheapest
 * underlying primitive that satisfies the requested capabilities; the actually
 * supported set may be a superset of the requested one. Invoking an operation
 * whose capability bit is not advertised by @ref get_supported_usage is a
 * precondition violation and throws @ref invalid_operation_error.
 */
class REXLIB_API event
{
public:
	event() noexcept;
	event(const event &other) = delete;
	event(event &&other) = delete;
	virtual ~event();

	event& operator=(const event &other) = delete;
	event& operator=(event &&other) = delete;

	/**
	 * @brief Get the set of operations supported by this event.
	 */
	virtual event_usage_flags get_supported_usage() const noexcept = 0;

	/**
	 * @brief Block the calling thread until the event is signaled.
	 *
	 * Suspends the host thread until the most recently recorded signal point on
	 * this event has been reached. Returns immediately if no signal has been
	 * recorded yet, since the initial state of the event is signaled.
	 *
	 * @note Requires @ref event_usage_flag_bits::host_wait.
	 * @throws invalid_operation_error if @ref event_usage_flag_bits::host_wait
	 * is not present.
	 */
	virtual void wait() const = 0;

	/**
	 * @brief Non-blocking query of the event's state.
	 *
	 * Reports whether the most recently recorded signal point has been reached
	 * by the queue, without blocking the calling thread. An event with no
	 * recorded signal is reported as signaled.
	 *
	 * @return @c true if the event has been signaled, @c false otherwise.
	 * 
	 * @note Requires @ref event_usage_flag_bits::host_query.
	 * @throws invalid_operation_error if @ref event_usage_flag_bits::host_query
	 * is not present.
	 */
	virtual bool is_signaled() const = 0;
};

} // namespace rexlib
