// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <memory>

namespace rexlib
{

class command;
class event;

/**
 * @brief Abstract command queue belonging to a @ref device.
 *
 * A @c command_queue represents an in-order stream of programs submitted
 * to a device (kernel launches, memory transfers...). Programs submitted to the
 * same queue execute in submission order; programs in different queues may
 * execute concurrently.
 */
class REXLIB_API command_queue
{
public:
	command_queue() noexcept;
	command_queue(const command_queue &other) = delete;
	command_queue(command_queue &&other) = delete;
	virtual ~command_queue();

	command_queue& operator=(const command_queue &other) = delete;
	command_queue& operator=(command_queue &&other) = delete;

	/**
	 * @brief Submit a program for execution on this queue.
	 *
	 * Schedules the program in @p cmd to run at the current point of
	 * this queue's execution timeline, operating on the supplied operands.
	 * The call returns once the work has been recorded or completed;
	 * execution is ordered with respect to other programs previously
	 * submitted to the same queue, and may proceed asynchronously with
	 * respect to the host thread depending on the backend.
	 *
	 * @param cmd The program to execute with its associated operands and
	 * workspaces.
	 * @pre @p cmd must hold a non-null program.
	 */
	virtual void submit(const command &cmd) = 0;

	/**
	 * @brief Record a signal on this queue for the given event.
	 *
	 * Schedules @p event to transition to the signaled state once this queue
	 * reaches the current point in its execution timeline. Any previously
	 * recorded signal point on @p event is superseded.
	 *
	 * Always supported on every event, regardless of
	 * @ref event::get_supported_usage. 
	 *
	 * @param event The event whose signal point is recorded at the current
	 * point of this queue's timeline.
	 * 
	 * @pre @p event must belong to the same device as this queue.
	 */
	virtual void signal(event &event) = 0;

	/**
	 * @brief Defer subsequent work on this queue until an event is signaled.
	 *
	 * Schedules this queue so that commands submitted after the call are
	 * deferred until the most recently recorded signal point on @p event has
	 * been reached. The call itself does not block the host thread.
	 *
	 * Requires @ref event_usage_flag_bits::device_wait on @p event. This
	 * queue may belong to a different device than the one that created the
	 * event only if @ref event_usage_flag_bits::cross_device_wait is also
	 * supported by @p event.
	 *
	 * @param event The event whose signal point this queue will wait for
	 * before executing any further submitted command.
	 * @throws invalid_operation_error if
	 * @ref event_usage_flag_bits::device_wait is not present on @p event.
	 * @throws invalid_operation_error if this queue does not belong to the
	 * same device as @p event and
	 * @ref event_usage_flag_bits::cross_device_wait is not set.
	 */
	virtual void wait(const event &event) = 0;
};

} // namespace rexlib
