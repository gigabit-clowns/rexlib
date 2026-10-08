// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Tells the host which commands of a queue have finished.
 *
 * A timeline gives an id to every command of its @ref command_queue. Only
 * the timeline that gave an id can interpret it.
 *
 * A timeline stays usable after its queue is destroyed.
 *
 * @see command_token
 */
class REXLIB_API command_timeline
{
public:
	command_timeline() noexcept;
	command_timeline(const command_timeline &other) = delete;
	command_timeline(command_timeline &&other) = delete;
	virtual ~command_timeline();

	command_timeline& operator=(const command_timeline &other) = delete;
	command_timeline& operator=(command_timeline &&other) = delete;

	/**
	 * @brief Block the calling thread until a command has finished.
	 *
	 * Returns immediately if the command has already finished.
	 *
	 * @param id The id this timeline gave to the command.
	 */
	virtual void wait(std::size_t id) const = 0;

	/**
	 * @brief Check whether a command has finished.
	 *
	 * Does not block the calling thread.
	 *
	 * @param id The id this timeline gave to the command.
	 * @return true The command has finished.
	 * @return false The command has not finished yet.
	 */
	virtual bool is_complete(std::size_t id) const = 0;
};

} // namespace rexlib
