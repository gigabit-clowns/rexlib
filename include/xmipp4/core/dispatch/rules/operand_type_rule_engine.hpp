// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_type_descriptor.hpp"
#include "operand_type_resolution.hpp"

#include <xmipp4/core/dispatch/operation_descriptor.hpp>
#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/platform/attributes.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>
#include <xmipp4/core/span.hpp>

namespace xmipp4
{

/**
 * @defgroup operand_type_rule_engine Operand type rule engine
 *
 * Runtime interpretation of a typing rule, lowered to descriptor tables.
 *
 * Every function here reports failures rather than throwing, so that a
 * backend can ask whether it supports a launch configuration without
 * paying for an exception and without an unsupported combination
 * surfacing as an error deep inside program construction. Turning a
 * failure into a diagnostic is a separate, explicit step.
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
 * @param operation_pivots Values supplied by the operation instance.
 * @return type_rule_resolution The resolution.
 */
REXLIB_API
type_rule_resolution resolve_pivots(
	span<const pivot_descriptor> pivots,
	span<const numerical_type> input_types,
	span<const numerical_type> operation_pivots
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
REXLIB_API
type_rule_resolution bind_free_pivots(
	span<const pivot_descriptor> pivots,
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
REXLIB_API
type_rule_resolution evaluate_slots(
	span<const slot_descriptor> slots,
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
REXLIB_API
type_rule_resolution check_slots(
	span<const slot_descriptor> slots,
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
XMIPP4_NORETURN REXLIB_API
void throw_type_rule_error(
	const operation_descriptor &descriptor,
	const type_rule_resolution &resolution,
	span<const slot_descriptor> slots,
	span<const pivot_descriptor> pivots
);

/** @} */

} // namespace xmipp4
