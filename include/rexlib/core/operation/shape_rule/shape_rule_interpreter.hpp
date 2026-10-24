// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/shape_rule_descriptor.hpp>

#include <rexlib/core/operation/operand_specification.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Interpreter of a shape rule lowered to tables.
 *
 * Deduces the shapes of the outputs of an operation from the shapes of
 * its inputs, and decides whether outputs supplied in advance have
 * admissible shapes. Data types are left alone.
 *
 * It knows nothing about operations or their parameters. Each call takes
 * the values the rule reads from the operation instance:
 *
 *   * the named axes, along which the core axes lie when the rule places
 *     them where the instance names them, empty otherwise;
 *   * whether an output with no core axes keeps the named axes with an
 *     extent of one;
 *   * the value of every extent variable the operation supplies, in
 *     declaration order.
 *
 * @see operand_shape_rule
 * @see shape_rule_descriptor_of
 */
class shape_rule_interpreter
{
public:
	/**
	 * @brief Construct an interpreter of a rule.
	 *
	 * @param rule The lowered rule. It must outlive the interpreter, which
	 * shape_rule_descriptor_of guarantees by holding it in static storage.
	 */
	REXLIB_API
	explicit shape_rule_interpreter(const shape_rule_descriptor &rule) noexcept;

	/**
	 * @brief Check the values an operation instance supplies.
	 *
	 * Checks what does not depend on the operands: how many axes are named
	 * and that none is named twice, and how many extents are supplied and
	 * that each lies in the domain of its variable.
	 *
	 * @param descriptor Description of the operation, used to name it when
	 * reporting a rejection.
	 * @param named_axes The named axes.
	 * @param operation_extents The extents the operation supplies.
	 *
	 * @throws std::invalid_argument When a value is not admissible.
	 */
	REXLIB_API
	void check_instance(
		const operation_descriptor &descriptor,
		span<const std::size_t> named_axes,
		span<const std::size_t> operation_extents
	) const;

	/**
	 * @brief Compute the shapes of the outputs the inputs call for.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param named_axes The named axes.
	 * @param keep_dimensions Whether an output with no core axes keeps the
	 * named axes.
	 * @param operation_extents The extents the operation supplies.
	 * @param canonical_outputs The outputs, one per output of the rule.
	 * Only their shapes are set.
	 * @param inputs The inputs, one per input of the rule.
	 *
	 * @throws std::invalid_argument When the inputs are not admissible.
	 * @throws std::out_of_range When a named axis is beyond the rank of the
	 * inputs.
	 */
	REXLIB_API
	void deduce(
		const operation_descriptor &descriptor,
		span<const std::size_t> named_axes,
		bool keep_dimensions,
		span<const std::size_t> operation_extents,
		span<operand_specification> canonical_outputs,
		span<const operand_specification> inputs
	) const;

	/**
	 * @brief Decide whether outputs supplied in advance have admissible
	 * shapes.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param named_axes The named axes.
	 * @param keep_dimensions Whether an output with no core axes keeps the
	 * named axes.
	 * @param operation_extents The extents the operation supplies.
	 * @param user_outputs The outputs supplied in advance, one per output
	 * of the rule.
	 * @param inputs The inputs, one per input of the rule.
	 *
	 * @throws std::invalid_argument When the inputs or an output are not
	 * admissible.
	 * @throws std::out_of_range When a named axis is beyond the rank of the
	 * inputs.
	 */
	REXLIB_API
	void accept(
		const operation_descriptor &descriptor,
		span<const std::size_t> named_axes,
		bool keep_dimensions,
		span<const std::size_t> operation_extents,
		span<const operand_specification> user_outputs,
		span<const operand_specification> inputs
	) const;

private:
	const shape_rule_descriptor *m_rule;
};

} // namespace rexlib
