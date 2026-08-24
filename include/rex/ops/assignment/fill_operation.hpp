// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/basic_operation.hpp>
#include <rex/core/numerical/scalar_value.hpp>
#include <rex/core/platform/dynamic_shared_object.h>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/elementwise_operation_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

namespace rex
{
namespace ops
{

REXLIB_DECLARE_OPERATION_TRAITS(
	fill,
	ops_component,
	REXLIB_OPERANDS("destination"),
	REXLIB_OPERANDS(),
	elementwise_operation_shape_policy,
	nullary_free_rule<>
);

/**
 * @brief Fill an array with a constant value.
 *
 * With no input to fix it, the element type is the one the destination
 * already carries.
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API fill_operation final
	: public trivial_operation<fill_operation, fill_operation_traits>
{
public:
	/**
	 * @brief Construct a fill operation by the value used to fill.
	 *
	 * @param fill_value Value used for filling.
	 */
	explicit fill_operation(const scalar_value &fill_value) noexcept;

	/**
	 * @brief Get the fill value.
	 *
	 * @return const scalar_value& The fill value.
	 */
	const scalar_value& get_fill_value() const noexcept;

private:
	REXLIB_STD_MEMBER_INTERFACE
	scalar_value m_fill_value;
};
REXLIB_END_TEMPLATE_BASE

} // namespace ops
} // namespace rex
