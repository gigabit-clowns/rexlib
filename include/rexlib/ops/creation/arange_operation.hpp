// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/parametric_operation.hpp>
#include <rexlib/core/numerical/scalar_value.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/policies/sequence_operation_shape_policy.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

#include <cstddef>

namespace rexlib
{
namespace ops
{

REXLIB_DECLARE_OPERATION_TRAITS(
	arange,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS(),
	sequence_operation_shape_policy,
	nullary_free_rule<real_arithmetic_type_domain>
);

/**
 * @brief Write the arithmetic progression `start + i * step`.
 *
 * The operation holds the first value, the step and how many elements to
 * write, rather than the half open range a caller thinks in: where the
 * progression stops is a question about values, and answering it is what
 * fixes the length. That answer belongs to whoever asked for the range,
 * because a shape has to be known before an operand can be allocated.
 *
 * The i-th element is computed from `i` rather than from the one before
 * it, so its error does not depend on how far into the sequence it sits.
 *
 * The domain excludes the complex types: a progression needs an ordering to
 * be asked where it stops, and the complex plane has none. Use @ref
 * linspace_operation, which is told its length outright, for a complex ramp.
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API arange_operation final
	: public parametric_operation<arange_operation, arange_operation_traits>
{
public:
	/**
	 * @brief Construct an arange operation.
	 *
	 * @param start Value of the first element.
	 * @param step Difference between one element and the next. It is not
	 * required to be non zero: a degenerate step writes a constant sequence,
	 * and it is the range to length conversion, not the operation, that has
	 * a reason to reject one.
	 * @param count Number of elements to write.
	 */
	arange_operation(
		const scalar_value &start,
		const scalar_value &step,
		std::size_t count
	);

	/**
	 * @brief Get the value of the first element.
	 *
	 * @return const scalar_value& The first value.
	 */
	const scalar_value& get_start() const noexcept;

	/**
	 * @brief Get the difference between consecutive elements.
	 *
	 * @return const scalar_value& The step.
	 */
	const scalar_value& get_step() const noexcept;

	/**
	 * @brief Get the number of elements written.
	 *
	 * @return std::size_t The element count.
	 */
	std::size_t get_count() const noexcept;

private:
	REXLIB_STD_MEMBER_INTERFACE
	scalar_value m_start;
	REXLIB_STD_MEMBER_INTERFACE
	scalar_value m_step;
};
REXLIB_END_TEMPLATE_BASE

} // namespace ops
} // namespace rexlib
