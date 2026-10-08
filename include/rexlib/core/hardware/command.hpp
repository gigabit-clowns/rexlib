// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "buffer.hpp"
#include "command_token.hpp"
#include "program.hpp"

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <memory>
#include <vector>

namespace rexlib
{

/**
 * @brief Specification of work to be executed on a @ref command_queue.
 *
 * A @c command binds a @ref program to its operands (outputs, inputs, and
 * scratch buffers) and to the commands it has to run after, and is passed to
 * @ref command_queue::submit for execution. It offers a fluent interface for
 * binding them, allowing the binding calls to be chained in a single
 * expression.
 *
 * A command owns what is bound to it. The buffers and the tokens stay alive
 * for as long as the command does.
 */
class command
{
public:
	/**
	 * @brief Construct a command with no program and empty bindings.
	 */
	REXLIB_API
	command() noexcept;

	/**
	 * @brief Construct a command associated with the given program.
	 *
	 * Bindings are initially empty; use @ref bind_outputs, @ref bind_inputs,
	 * @ref bind_scratch and @ref bind_dependencies to populate them before
	 * submission.
	 *
	 * @param program The program to execute. May be @c nullptr, but
	 * @ref command_queue::submit requires a non-null program.
	 */
	REXLIB_API
	explicit command(std::shared_ptr<const program> program) noexcept;

	REXLIB_API
	command(const command &other);
	REXLIB_API
	command(command &&other) noexcept;
	REXLIB_API
	~command();

	REXLIB_API
	command& operator=(const command &other);
	REXLIB_API
	command& operator=(command &&other) noexcept;

	/**
	 * @brief Bind the output buffers for this command.
	 *
	 * Replaces the output buffers bound before.
	 *
	 * @param outputs The output buffer handles. May be empty.
	 * @return Reference to @c *this to allow method chaining.
	 */
	REXLIB_API
	command&
	bind_outputs(std::vector<std::shared_ptr<buffer>> outputs) noexcept;

	/**
	 * @brief Bind the input buffers for this command.
	 *
	 * Replaces the input buffers bound before.
	 *
	 * @param inputs The input buffer handles. May be empty.
	 * @return Reference to @c *this to allow method chaining.
	 */
	REXLIB_API
	command&
	bind_inputs(std::vector<std::shared_ptr<const buffer>> inputs) noexcept;

	/**
	 * @brief Bind the scratch buffers for this command.
	 *
	 * Scratch buffers provide temporary workspace required by the program
	 * (see @ref program::get_scratch_requirements). They must be bound
	 * in the same order as the requirements returned by that method.
	 *
	 * Replaces the scratch buffers bound before.
	 *
	 * @param scratch The scratch buffer handles. May be empty if the
	 * program has no scratch requirements.
	 * @return Reference to @c *this to allow method chaining.
	 */
	REXLIB_API
	command&
	bind_scratch(std::vector<std::shared_ptr<buffer>> scratch) noexcept;

	/**
	 * @brief Bind the commands this command has to run after.
	 *
	 * Replaces the dependencies bound before.
	 *
	 * @param dependencies Tokens of the commands to run after. May be empty.
	 * An empty token is allowed and adds nothing.
	 * @return Reference to @c *this to allow method chaining.
	 */
	REXLIB_API
	command&
	bind_dependencies(std::vector<command_token> dependencies) noexcept;

	/**
	 * @brief Get the program associated with this command.
	 *
	 * @return The program, or @c nullptr if no program was set.
	 */
	REXLIB_API
	const std::shared_ptr<const program>& get_program() const noexcept;

	/**
	 * @brief Get the bound output buffers.
	 *
	 * @return Span over the output buffer handles this command holds; empty
	 * if none were bound. Valid until they are bound again or the command is
	 * destroyed.
	 */
	REXLIB_API
	span<const std::shared_ptr<buffer>> get_outputs() const noexcept;

	/**
	 * @brief Get the bound input buffers.
	 *
	 * @return Span over the input buffer handles this command holds; empty
	 * if none were bound. Valid until they are bound again or the command is
	 * destroyed.
	 */
	REXLIB_API
	span<const std::shared_ptr<const buffer>> get_inputs() const noexcept;

	/**
	 * @brief Get the bound scratch buffers.
	 *
	 * @return Span over the scratch buffer handles this command holds; empty
	 * if none were bound. Valid until they are bound again or the command is
	 * destroyed.
	 */
	REXLIB_API
	span<const std::shared_ptr<buffer>> get_scratch() const noexcept;

	/**
	 * @brief Get the bound dependencies.
	 *
	 * @return Span over the tokens of the commands to run after; empty if
	 * none were bound. Valid until they are bound again or the command is
	 * destroyed.
	 */
	REXLIB_API
	span<const command_token> get_dependencies() const noexcept;

private:
	std::shared_ptr<const program> m_program;
	std::vector<std::shared_ptr<buffer>> m_outputs;
	std::vector<std::shared_ptr<const buffer>> m_inputs;
	std::vector<std::shared_ptr<buffer>> m_scratch;
	std::vector<command_token> m_dependencies;
};

} // namespace rexlib
