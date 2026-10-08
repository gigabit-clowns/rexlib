// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <memory>

namespace rexlib
{

class command_timeline;

/**
 * @brief Stands for one command submitted to a @ref command_queue.
 *
 * A token pairs a @ref command_timeline with the id that timeline gave to a
 * command. It is a value: it can be copied freely, and every copy stands for
 * the same command.
 *
 * A token without a timeline stands for no command. Such a token is always
 * complete.
 */
class command_token
{
public:
	/**
	 * @brief Construct a token that stands for no command.
	 */
	REXLIB_API
	command_token() noexcept;

	/**
	 * @brief Construct a token from its components.
	 *
	 * @param timeline The timeline that gave the id. A null timeline makes
	 * the token stand for no command.
	 * @param id The id of the command in @p timeline.
	 */
	REXLIB_API
	command_token(
		std::shared_ptr<const command_timeline> timeline,
		std::size_t id
	) noexcept;

	REXLIB_API
	command_token(const command_token &other) noexcept;
	REXLIB_API
	command_token(command_token &&other) noexcept;
	REXLIB_API
	~command_token();

	REXLIB_API
	command_token& operator=(const command_token &other) noexcept;
	REXLIB_API
	command_token& operator=(command_token &&other) noexcept;

	/**
	 * @brief Get the timeline that gave the id.
	 *
	 * @return The timeline, or null if this stands for no command.
	 */
	REXLIB_API
	const std::shared_ptr<const command_timeline>&
	get_timeline() const noexcept;

	/**
	 * @brief Get the id of the command in its timeline.
	 *
	 * @return The id. It has no meaning if this stands for no command.
	 */
	REXLIB_API
	std::size_t get_id() const noexcept;

	/**
	 * @brief Check whether the command has finished.
	 *
	 * Does not block the calling thread.
	 *
	 * @return true The command has finished, or this stands for no command.
	 * @return false The command has not finished yet.
	 */
	REXLIB_API
	bool is_complete() const;

	/**
	 * @brief Block the calling thread until the command has finished.
	 *
	 * Returns immediately if this stands for no command.
	 */
	REXLIB_API
	void wait() const;

private:
	std::shared_ptr<const command_timeline> m_timeline;
	std::size_t m_id;
};

} // namespace rexlib
