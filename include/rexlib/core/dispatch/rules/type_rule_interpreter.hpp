// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_type_descriptor.hpp"

#include <rexlib/core/operation/contract/operand_specification.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

namespace rexlib
{

/**
 * @brief Interpreter of a typing rule lowered to tables.
 *
 * Deduces the data types of the outputs of an operation from the data
 * types of its inputs, and decides whether outputs supplied in advance have
 * admissible data types. Shapes are left alone.
 *
 * It reads the descriptor tables a rule lowers to rather than the rule
 * itself, so a catalog of hundreds of operations shares one implementation
 * instead of instantiating one per rule. Each call takes the data types the
 * operation instance supplies, in the order the pivots reading them are
 * declared.
 *
 * `deduce` resolves the rule's pivots from the input types, checks every
 * input against its slot and evaluates the output slots. `accept` binds the
 * rule's free pivots from the outputs supplied in advance and then checks
 * every operand, which admits the looser outputs a converting operation
 * needs.
 *
 * @see operand_type_rule
 */
class type_rule_interpreter
{
public:
	/**
	 * @brief Construct an interpreter of the tables of a rule.
	 *
	 * The tables must outlive the interpreter, which the table templates
	 * guarantee by holding them in static storage.
	 *
	 * @param pivots The rule's pivot table.
	 * @param output_slots The rule's output slot table.
	 * @param input_slots The rule's input slot table.
	 */
	REXLIB_API
	type_rule_interpreter(
		span<const pivot_descriptor> pivots,
		span<const slot_descriptor> output_slots,
		span<const slot_descriptor> input_slots
	) noexcept;

	/**
	 * @brief Check the data types an operation instance supplies.
	 *
	 * @param descriptor Description of the operation, used to name it when
	 * reporting a rejection.
	 * @param operation_pivots The data types the operation supplies.
	 *
	 * @throws std::invalid_argument When there are not as many as the rule
	 * reads, or one lies outside the domain of its pivot.
	 */
	REXLIB_API
	void check_instance(
		const operation_descriptor &descriptor,
		span<const numerical_type> operation_pivots
	) const;

	/**
	 * @brief Compute the data types of the outputs the inputs call for.
	 *
	 * A data type the inputs do not determine is left as
	 * numerical_type::unknown.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param operation_pivots The data types the operation supplies.
	 * @param canonical_outputs The outputs, one per output slot. Only their
	 * data types are set.
	 * @param inputs The inputs, one per input slot.
	 *
	 * @throws std::invalid_argument When the inputs are not admissible.
	 */
	REXLIB_API
	void deduce(
		const operation_descriptor &descriptor,
		span<const numerical_type> operation_pivots,
		span<operand_specification> canonical_outputs,
		span<const operand_specification> inputs
	) const;

	/**
	 * @brief Decide whether outputs supplied in advance have admissible data
	 * types.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param operation_pivots The data types the operation supplies.
	 * @param user_outputs The outputs supplied in advance, one per output
	 * slot.
	 * @param inputs The inputs, one per input slot.
	 *
	 * @throws std::invalid_argument When the inputs or an output are not
	 * admissible.
	 */
	REXLIB_API
	void accept(
		const operation_descriptor &descriptor,
		span<const numerical_type> operation_pivots,
		span<const operand_specification> user_outputs,
		span<const operand_specification> inputs
	) const;

private:
	span<const pivot_descriptor> m_pivots;
	span<const slot_descriptor> m_output_slots;
	span<const slot_descriptor> m_input_slots;

	span<const slot_descriptor> get_slots_for_report(
		bool output_operands
	) const noexcept;

	void check_operand_counts(
		const operation_descriptor &descriptor,
		std::size_t output_count,
		std::size_t input_count
	) const;
};

} // namespace rexlib
