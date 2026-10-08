// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <memory>

namespace rexlib
{

class command_timeline;

/**
 * @brief Identifies a command submitted to a @ref command_queue.
 *
 * A token pairs a @ref command_timeline with the id that timeline gave to a
 * command. It is a value: it can be copied freely, and every copy identifies
 * the same command.
 *
 * @par Empty state
 * A token without a timeline is @em empty: it identifies no command. An
 * empty token is always complete, and waiting for it returns at once.
 */
class command_token
{
public:
	/**
	 * @brief Construct an empty token.
	 */
	REXLIB_API
	command_token() noexcept;

	/**
	 * @brief Construct a token from its components.
	 *
	 * @param timeline The timeline that gave the id. A null timeline makes
	 * the token empty.
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
	 * @return The timeline, or null if this is empty.
	 */
	REXLIB_API
	const std::shared_ptr<const command_timeline>&
	get_timeline() const noexcept;

	/**
	 * @brief Get the id of the command in its timeline.
	 *
	 * @return The id. It has no meaning if this is empty.
	 */
	REXLIB_API
	std::size_t get_id() const noexcept;

	/**
	 * @brief Check whether this is empty.
	 *
	 * @return true This identifies no command.
	 * @return false This identifies a command.
	 */
	REXLIB_API
	bool is_empty() const noexcept;

	/**
	 * @brief Check whether the command has finished.
	 *
	 * Does not block the calling thread.
	 *
	 * @return true The command has finished, or this is empty.
	 * @return false The command has not finished yet.
	 */
	REXLIB_API
	bool is_complete() const;

	/**
	 * @brief Block the calling thread until the command has finished.
	 *
	 * Returns immediately if this is empty.
	 */
	REXLIB_API
	void wait() const;

private:
	std::shared_ptr<const command_timeline> m_timeline;
	std::size_t m_id;
};

} // namespace rexlib
