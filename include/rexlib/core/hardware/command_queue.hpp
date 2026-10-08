// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/command.hpp>
#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <memory>

namespace rexlib
{

class event;

/**
 * @brief Abstract command queue belonging to a @ref device.
 *
 * A @c command_queue runs the commands submitted to it on a device (kernel
 * launches, memory transfers...). A command runs after the commands it
 * depends on. Commands that do not depend on each other, directly or
 * through other commands, may run in any order or at the same time,
 * whichever queue they were submitted to.
 *
 * @see command::bind_dependencies
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
	 * @brief Submit a command for execution on this queue.
	 *
	 * Schedules the program in @p cmd to run on the supplied operands, once
	 * every command it depends on has finished. The call returns once the
	 * work has been recorded or completed; it may proceed asynchronously
	 * with respect to the host thread depending on the backend.
	 *
	 * A dependency may stand for a command of any queue. If this queue
	 * cannot wait for it on its device, the call blocks the calling thread
	 * until that command has finished.
	 *
	 * @param cmd The program to execute with its associated operands,
	 * workspaces and dependencies.
	 * @return The token that stands for the submitted command. It stands
	 * for no command if the command has finished when the call returns.
	 * @pre @p cmd must hold a non-null program.
	 */
	virtual command_token submit(command cmd) = 0;

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
