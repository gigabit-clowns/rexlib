// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Describes the operand count of an operation.
 */
class operation_arity
{
public:
	/**
	 * @brief Construct an arity with no outputs and no inputs.
	 */
	REXLIB_API
	operation_arity() noexcept;

	/**
	 * @brief Construct an arity from its operand counts.
	 *
	 * @param output_count The number of output operands.
	 * @param input_count The number of input operands.
	 */
	REXLIB_API
	operation_arity(
		std::size_t output_count,
		std::size_t input_count
	) noexcept;

	operation_arity(const operation_arity &other) = default;
	operation_arity(operation_arity &&other) noexcept = default;
	~operation_arity() = default;

	operation_arity& operator=(const operation_arity &other) = default;
	operation_arity& operator=(operation_arity &&other) noexcept = default;

	/**
	 * @brief Get the number of output operands.
	 *
	 * @return std::size_t The output count.
	 */
	REXLIB_API
	std::size_t get_output_count() const noexcept;

	/**
	 * @brief Get the number of input operands.
	 *
	 * @return std::size_t The input count.
	 */
	REXLIB_API
	std::size_t get_input_count() const noexcept;

	/**
	 * @brief Create the arity of a nullary operation.
	 *
	 * A nullary operation consumes no input operands and produces one output
	 * operand.
	 *
	 * @return operation_arity Arity with one output and no inputs.
	 */
	REXLIB_API
	static operation_arity nullary() noexcept;

	/**
	 * @brief Create the arity of a unary operation.
	 *
	 * A unary operation consumes one input operand and produces one output
	 * operand.
	 *
	 * @return operation_arity Arity with one output and one input.
	 */
	REXLIB_API
	static operation_arity unary() noexcept;

	/**
	 * @brief Create the arity of a binary operation.
	 *
	 * A binary operation consumes two input operands and produces one output
	 * operand.
	 *
	 * @return operation_arity Arity with one output and two inputs.
	 */
	REXLIB_API
	static operation_arity binary() noexcept;

	/**
	 * @brief Create the arity of a ternary operation.
	 *
	 * A ternary operation consumes three input operands and produces one
	 * output operand.
	 *
	 * @return operation_arity Arity with one output and three inputs.
	 */
	REXLIB_API
	static operation_arity ternary() noexcept;

	friend bool operator==(
		const operation_arity &lhs,
		const operation_arity &rhs
	) noexcept
	{
		return
			lhs.get_output_count() == rhs.get_output_count() &&
			lhs.get_input_count() == rhs.get_input_count();
	}

	friend bool operator!=(
		const operation_arity &lhs,
		const operation_arity &rhs
	) noexcept
	{
		return !(lhs == rhs);
	}

private:
	std::size_t m_output_count;
	std::size_t m_input_count;
};

} // namespace rexlib
