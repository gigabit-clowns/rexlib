// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operand_specification.hpp>
#include <rexlib/core/operation/operation_arity.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

namespace rexlib
{

/**
 * @brief Interface deciding the shape and data type of the operands of an
 * operation.
 *
 * The signature has two roles:
 *
 *   * `deduce` computes the output operands the inputs call for, and checks
 *     the inputs along the way.
 *   * `accept` decides whether output operands supplied in advance are
 *     admissible for the inputs.
 *
 * It works on operand_specification values, which hold a shape and a data
 * type, not on the operand_descriptor a program is built against, which
 * also holds a memory layout and resource.
 */
class REXLIB_API operation_signature
{
public:
	operation_signature() noexcept;
	operation_signature(const operation_signature &other) = delete;
	operation_signature(operation_signature &&other) = delete;
	virtual ~operation_signature();

	operation_signature&
	operator=(const operation_signature &other) = delete;
	operation_signature&
	operator=(operation_signature &&other) = delete;

	/**
	 * @brief Compute the output operands the inputs call for.
	 *
	 * A data type the inputs do not determine is left as
	 * numerical_type::unknown, for an output supplied in advance to fix.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param canonical_outputs Output buffer, sized to the operation's
	 * output arity.
	 * @param inputs The input operands.
	 *
	 * @throws std::invalid_argument When the inputs are not admissible.
	 * @throws std::out_of_range When the signature names an axis beyond the
	 * rank of the operands.
	 */
	virtual void deduce(
		const operation_descriptor &descriptor,
		span<operand_specification> canonical_outputs,
		span<const operand_specification> inputs
	) const = 0;

	/**
	 * @brief Decide whether output operands supplied in advance are
	 * admissible.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param user_outputs The output operands supplied in advance.
	 * @param inputs The input operands.
	 *
	 * @throws std::invalid_argument When an output is not admissible.
	 * @throws std::out_of_range When the signature names an axis beyond the
	 * rank of the operands.
	 */
	virtual void accept(
		const operation_descriptor &descriptor,
		span<const operand_specification> user_outputs,
		span<const operand_specification> inputs
	) const = 0;

	/**
	 * @brief Get the number of output and input operands.
	 *
	 * @return operation_arity The operand counts `deduce` and `accept`
	 * take.
	 */
	virtual operation_arity get_arity() const noexcept = 0;
};

} // namespace rexlib
