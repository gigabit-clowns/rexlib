// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/command.hpp>
#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <memory>

namespace rexlib
{

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
	 * @return The token of the submitted command. It is empty if the
	 * command has finished when the call returns.
	 * @pre @p cmd must hold a non-null program.
	 */
	virtual command_token submit(command cmd) = 0;
};

} // namespace rexlib
