// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_specification.hpp"
#include "operation_descriptor.hpp"

#include "../platform/dynamic_shared_object.h"
#include "../span.hpp"

namespace rexlib
{

/**
 * @brief Interface deciding the shape and data type of the operands of an
 * operation.
 *
 * The contract has two roles:
 *
 *   * `deduce` computes the output operands the inputs call for, and checks
 *     the inputs along the way.
 *   * `accept` decides whether output operands supplied in advance are
 *     admissible for the inputs.
 */
class REXLIB_API operation_contract
{
public:
	operation_contract() noexcept;
	operation_contract(const operation_contract &other) = delete;
	operation_contract(operation_contract &&other) = delete;
	virtual ~operation_contract();

	operation_contract&
	operator=(const operation_contract &other) = delete;
	operation_contract&
	operator=(operation_contract &&other) = delete;

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
	 * @throws std::out_of_range When the contract names an axis beyond the
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
	 * @param canonical_outputs The output operands `deduce` computed.
	 * @param inputs The input operands.
	 *
	 * @throws std::invalid_argument When an output is not admissible.
	 * @throws std::out_of_range When the contract names an axis beyond the
	 * rank of the operands.
	 */
	virtual void accept(
		const operation_descriptor &descriptor,
		span<const operand_specification> user_outputs,
		span<const operand_specification> canonical_outputs,
		span<const operand_specification> inputs
	) const = 0;
};

} // namespace rexlib
