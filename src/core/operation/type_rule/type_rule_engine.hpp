// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/type_pivot_descriptor.hpp>
#include <rexlib/core/operation/type_rule/type_rule_resolution.hpp>
#include <rexlib/core/operation/type_rule/type_slot_descriptor.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/platform/attributes.hpp>
#include <rexlib/core/span.hpp>

namespace rexlib
{

/**
 * @defgroup type_rule_engine Type rule engine
 *
 * Runtime interpretation of a typing rule, lowered to descriptor tables.
 *
 * Every function here reports a failure as a type_rule_resolution.
 * throw_type_rule_error turns one into an exception.
 *
 * @{
 */

/**
 * @brief Resolve the pivots a rule fixes from its inputs.
 *
 * Pivots sourced from an input operand or from the operation are read and
 * validated against their domain. Free pivots are left as
 * numerical_type::unknown unless they declare a default pivot, in which
 * case they adopt its value.
 *
 * @param pivots The rule's pivot table.
 * @param input_types Data types of the input operands.
 * @param operation_data_types Data types the operation instance supplies.
 * @return type_rule_resolution The resolution.
 */
type_rule_resolution resolve_pivots(
	span<const type_pivot_descriptor> pivots,
	span<const numerical_type> input_types,
	span<const numerical_type> operation_data_types
) noexcept;

/**
 * @brief Bind the free pivots of a rule from user supplied outputs.
 *
 * Called when the user pre-allocated the outputs, so that the types they
 * chose determine the pivots the inputs did not fix.
 *
 * @param pivots The rule's pivot table.
 * @param partial Resolution produced by resolve_pivots.
 * @param user_output_types Data types of the user supplied outputs.
 * @return type_rule_resolution The resolution with every pivot bound.
 */
type_rule_resolution bind_free_pivots(
	span<const type_pivot_descriptor> pivots,
	const type_rule_resolution &partial,
	span<const numerical_type> user_output_types
) noexcept;

/**
 * @brief Evaluate a rule's slots to produce operand types.
 *
 * @param slots The slot table of one operand kind.
 * @param resolution A successful resolution.
 * @param types Output buffer, one entry per slot.
 * @return type_rule_resolution A successful resolution, or the reason a
 * transform could not be evaluated.
 */
type_rule_resolution evaluate_slots(
	span<const type_slot_descriptor> slots,
	const type_rule_resolution &resolution,
	span<numerical_type> types,
	bool output_operands
) noexcept;

/**
 * @brief Check operand types against a rule's slots.
 *
 * @param slots The slot table of one operand kind.
 * @param resolution A successful resolution.
 * @param types Data types of the operands, one per slot.
 * @param output_operands Whether @p types holds output operands. Only
 * affects how a failure is reported.
 * @return type_rule_resolution A successful resolution, or the reason an
 * operand was rejected.
 */
type_rule_resolution check_slots(
	span<const type_slot_descriptor> slots,
	const type_rule_resolution &resolution,
	span<const numerical_type> types,
	bool output_operands
) noexcept;

/**
 * @brief Turn a failed resolution into an exception.
 *
 * @param descriptor Description of the operation, used to name it and its
 * operands.
 * @param resolution The failed resolution. Must not be successful.
 * @param slots Slot table of the operand kind the failure refers to. May
 * be empty when the failure is not attributed to a slot.
 * @param pivots The rule's pivot table.
 * @throws std::invalid_argument Always.
 */
REXLIB_NORETURN
void throw_type_rule_error(
	const operation_descriptor &descriptor,
	const type_rule_resolution &resolution,
	span<const type_slot_descriptor> slots,
	span<const type_pivot_descriptor> pivots
);

/** @} */

} // namespace rexlib
