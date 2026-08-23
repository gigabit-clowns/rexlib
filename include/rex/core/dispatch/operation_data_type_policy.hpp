// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operation_descriptor.hpp"

#include <rex/core/span.hpp>
#include <rex/core/numerical/numerical_type.hpp>
#include <rex/core/platform/dynamic_shared_object.h>

namespace rex
{

/**
 * @brief Interface describing the deduction and admissibility of array
 * data types for an operation.
 *
 * The contract is split in two roles:
 *
 *   * `deduce` is the single source of truth: from the input data types
 *     alone, it produces the canonical output data types that the policy
 *     would assign, and along the way performs every input-side check the
 *     policy requires.
 *   * `accept` is invoked only when the user supplies pre-allocated
 *     outputs. It decides whether those user-supplied output data types
 *     are admissible given the canonical ones produced by `deduce`. The
 *     default implementation requires equality with the canonical types;
 *     override only for policies that admit user outputs other than the
 *     canonical one (e.g. type-converting copies).
 */
class REXLIB_API operation_data_type_policy
{
public:
	operation_data_type_policy() noexcept;
	operation_data_type_policy(
		const operation_data_type_policy &other
	) = delete;
	operation_data_type_policy(operation_data_type_policy &&other) = delete;
	virtual ~operation_data_type_policy();

	operation_data_type_policy&
	operator=(const operation_data_type_policy &other) = delete;
	operation_data_type_policy&
	operator=(operation_data_type_policy &&other) = delete;

	/**
	 * @brief Compute canonical output data types from the input data types.
	 *
	 * Performs all input-side validation along the way. After this call,
	 * @p canonical_output_types holds the data types the policy would
	 * produce if the user had not pre-allocated outputs.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param canonical_output_types Output buffer, sized to the operation's
	 * output arity. Will be filled with the canonical data types.
	 * @param input_types Data types of the input operands.
	 */
	virtual void deduce(
		const operation_descriptor &descriptor,
		span<numerical_type> canonical_output_types,
		span<const numerical_type> input_types
	) const = 0;

	/**
	 * @brief Decide whether user-supplied output data types are admissible.
	 *
	 * Called only when the user has pre-allocated outputs. By the time
	 * this is invoked, `deduce` has already produced
	 * @p canonical_output_types, so input-side validation has happened.
	 * Throws on rejection.
	 *
	 * The default implementation requires @p user_output_types to be
	 * equal to @p canonical_output_types; override only for policies
	 * that admit user outputs other than the canonical one (e.g. when the
	 * program performs a type conversion).
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param user_output_types Data types of the user-supplied outputs.
	 * @param canonical_output_types Data types produced by `deduce`.
	 * @param input_types Data types of the input operands.
	 */
	virtual void accept(
		const operation_descriptor &descriptor,
		span<const numerical_type> user_output_types,
		span<const numerical_type> canonical_output_types,
		span<const numerical_type> input_types
	) const;
};

} // namespace rex
