// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../hardware/command_token.hpp"
#include "../platform/dynamic_shared_object.h"
#include "../system/access_flags.hpp"

#include <mutex>
#include <vector>

namespace rexlib
{

/**
 * @brief Tracks the commands that access a piece of memory.
 *
 * The tracker holds the token of the last command that writes the memory and
 * the tokens of the commands that read it since. It tells an access which of
 * them it has to wait for: a read waits for the write, and a write waits for
 * the write and for every read.
 *
 * An access that includes a write is a write.
 *
 * @par Thread safety
 * Every method may be called concurrently.
 */
class access_hazard_tracker
{
public:
	/**
	 * @brief Construct a tracker with no command recorded.
	 */
	REXLIB_API
	access_hazard_tracker();
	access_hazard_tracker(const access_hazard_tracker &other) = delete;
	access_hazard_tracker(access_hazard_tracker &&other) = delete;
	REXLIB_API
	~access_hazard_tracker();

	access_hazard_tracker&
	operator=(const access_hazard_tracker &other) = delete;
	access_hazard_tracker&
	operator=(access_hazard_tracker &&other) = delete;

	/**
	 * @brief Record a command that accesses the memory.
	 *
	 * A read is added to the reads recorded since the last write. A write
	 * replaces everything recorded before it, so the command has to wait for
	 * the tokens that @ref collect gives a write.
	 *
	 * @param token The token of the command. A token that stands for no
	 * command is not kept.
	 * @param access How the command accesses the memory.
	 *
	 * @throws std::invalid_argument If @p access is empty.
	 */
	REXLIB_API
	void add(command_token token, access_flags access);

	/**
	 * @brief Get the tokens that an access has to wait for.
	 *
	 * @param access How the memory is going to be accessed.
	 * @param tokens Output parameter where the tokens are appended.
	 *
	 * @throws std::invalid_argument If @p access is empty.
	 */
	REXLIB_API
	void collect(
		access_flags access,
		std::vector<command_token> &tokens
	) const;

	/**
	 * @brief Block the calling thread until an access can take place.
	 *
	 * Waits for the tokens that @ref collect gives @p access.
	 *
	 * @param access How the memory is going to be accessed.
	 *
	 * @throws std::invalid_argument If @p access is empty.
	 */
	REXLIB_API
	void wait(access_flags access) const;

private:
	mutable std::mutex m_mutex;
	command_token m_write;
	std::vector<command_token> m_reads;

	void drop_complete_reads();
};

} // namespace rexlib
