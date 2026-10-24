// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/type_rule_descriptor.hpp>
#include <rexlib/core/operation/type_rule/type_rule_resolution.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/operation/operand_specification.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/platform/attributes.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Interpreter of a typing rule lowered to a descriptor.
 *
 * Deduces the data types of the outputs of an operation from the data
 * types of its inputs, and decides whether outputs supplied in advance have
 * admissible data types. Shapes are left alone.
 *
 * Each call takes the data types the operation instance supplies, in the
 * order the pivots reading them are declared.
 *
 * `deduce` resolves the rule's pivots from the input types, checks every
 * input against its slot and evaluates the output slots. `resolve` binds
 * the rule's free pivots from the outputs as well and checks every operand,
 * which admits the looser outputs a converting operation needs. It reports
 * a rejection as a type_rule_resolution, and `throw_error` turns that into
 * an exception. `accept` does both.
 *
 * @see operand_type_rule
 */
class type_rule_interpreter
{
public:
	/**
	 * @brief Construct an interpreter of a rule.
	 *
	 * @param rule The rule. It must outlive the interpreter, which
	 * type_rule_descriptor_of guarantees by holding it in static storage.
	 */
	REXLIB_API
	explicit type_rule_interpreter(const type_rule_descriptor &rule) noexcept;

	/**
	 * @brief Check the data types an operation instance supplies.
	 *
	 * @param descriptor Description of the operation, used to name it when
	 * reporting a rejection.
	 * @param operation_data_types The data types the operation supplies.
	 *
	 * @throws std::invalid_argument When there are not as many as the rule
	 * reads, or one lies outside the domain of its pivot.
	 */
	REXLIB_API
	void check_instance(
		const operation_descriptor &descriptor,
		span<const numerical_type> operation_data_types
	) const;

	/**
	 * @brief Compute the data types of the outputs the inputs call for.
	 *
	 * A data type the inputs do not determine is left as
	 * numerical_type::unknown.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param operation_data_types The data types the operation supplies.
	 * @param canonical_outputs The outputs, one per output slot. Only their
	 * data types are set.
	 * @param inputs The inputs, one per input slot.
	 *
	 * @throws std::invalid_argument When the inputs are not admissible.
	 */
	REXLIB_API
	void deduce(
		const operation_descriptor &descriptor,
		span<const numerical_type> operation_data_types,
		span<operand_specification> canonical_outputs,
		span<const operand_specification> inputs
	) const;

	/**
	 * @brief Decide whether outputs supplied in advance have admissible data
	 * types.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param operation_data_types The data types the operation supplies.
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
		span<const numerical_type> operation_data_types,
		span<const operand_specification> user_outputs,
		span<const operand_specification> inputs
	) const;

	/**
	 * @brief Resolve the rule against concrete operand data types.
	 *
	 * @param operation_data_types The data types the operation supplies.
	 * @param output_types The data type of every output.
	 * @param input_types The data type of every input.
	 * @return type_rule_resolution The resolved pivots, or the reason the
	 * operands are not admissible.
	 */
	REXLIB_API
	type_rule_resolution resolve(
		span<const numerical_type> operation_data_types,
		span<const numerical_type> output_types,
		span<const numerical_type> input_types
	) const noexcept;

	/**
	 * @brief Turn a failed resolution into an exception.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands.
	 * @param resolution A failed resolution of this rule.
	 *
	 * @throws std::invalid_argument Always.
	 */
	REXLIB_NORETURN REXLIB_API
	void throw_error(
		const operation_descriptor &descriptor,
		const type_rule_resolution &resolution
	) const;

private:
	const type_rule_descriptor *m_rule;

	type_rule_resolution check_operand_counts(
		std::size_t output_count,
		std::size_t input_count
	) const noexcept;
};

} // namespace rexlib
