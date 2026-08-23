// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "buffer.hpp"
#include "program.hpp"

#include <xmipp4/core/platform/dynamic_shared_object.h>
#include <xmipp4/core/span.hpp>

#include <memory>

namespace xmipp4
{

/**
 * @brief Specification of work to be executed on a @ref command_queue.
 *
 * A @c command binds a @ref program to its operands (outputs, inputs, and
 * scratch buffers) and is passed to @ref command_queue::submit for execution.
 * It offers a fluent interface for binding operands, allowing all three
 * binding calls to be chained in a single expression.
 *
 * This is a short-lived descriptor: it does not allocate memory. Instead, it
 * stores non-owning @ref span views over caller-managed buffer arrays. The
 * caller must ensure that every bound span and all buffers it references
 * remain valid for the entire lifetime of the @c command and until
 * @ref command_queue::submit returns.
 */
class command
{
public:
	/**
	 * @brief Construct a command with no program and empty operand bindings.
	 */
	REXLIB_API
	command() noexcept;

	/**
	 * @brief Construct a command associated with the given program.
	 *
	 * Operand bindings are initially empty; use @ref bind_outputs,
	 * @ref bind_inputs, and @ref bind_scratch to populate them before
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
	 * Stores a non-owning view of @p outputs. The caller must keep the span
	 * contents alive until @ref command_queue::submit returns.
	 *
	 * @param outputs Span of output buffer handles. May be empty.
	 * @return Reference to @c *this to allow method chaining.
	 */
	REXLIB_API
	command&
	bind_outputs(span<const std::shared_ptr<buffer>> outputs) noexcept;

	/**
	 * @brief Bind the input buffers for this command.
	 *
	 * Stores a non-owning view of @p inputs. The caller must keep the span
	 * contents alive until @ref command_queue::submit returns.
	 *
	 * @param inputs Span of input buffer handles. May be empty.
	 * @return Reference to @c *this to allow method chaining.
	 */
	REXLIB_API
	command&
	bind_inputs(span<const std::shared_ptr<const buffer>> inputs) noexcept;

	/**
	 * @brief Bind the scratch buffers for this command.
	 *
	 * Scratch buffers provide temporary workspace required by the program
	 * (see @ref program::get_scratch_requirements). They must be bound
	 * in the same order as the requirements returned by that method.
	 *
	 * Stores a non-owning view of @p scratch. The caller must keep the span
	 * contents alive until @ref command_queue::submit returns.
	 *
	 * @param scratch Span of scratch buffer handles. May be empty if the
	 * program has no scratch requirements.
	 * @return Reference to @c *this to allow method chaining.
	 */
	REXLIB_API
	command&
	bind_scratch(span<const std::shared_ptr<buffer>> scratch) noexcept;

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
	 * @return Non-owning span of output buffer handles; empty if none were
	 * bound.
	 */
	REXLIB_API
	span<const std::shared_ptr<buffer>> get_outputs() const noexcept;

	/**
	 * @brief Get the bound input buffers.
	 *
	 * @return Non-owning span of input buffer handles; empty if none were
	 * bound.
	 */
	REXLIB_API
	span<const std::shared_ptr<const buffer>> get_inputs() const noexcept;

	/**
	 * @brief Get the bound scratch buffers.
	 *
	 * @return Non-owning span of scratch buffer handles; empty if none were
	 * bound.
	 */
	REXLIB_API
	span<const std::shared_ptr<buffer>> get_scratch() const noexcept;

private:
	std::shared_ptr<const program> m_program;
	span<const std::shared_ptr<buffer>> m_outputs;
	span<const std::shared_ptr<const buffer>> m_inputs;
	span<const std::shared_ptr<buffer>> m_scratch;
};

} // namespace xmipp4
