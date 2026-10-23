// SPDX-License-Identifier: GPL-3.0-only

#include "rule_contract.hpp"

#include <rexlib/core/dispatch/rules/operand_type_descriptor.hpp>
#include <rexlib/core/dispatch/rules/shape_rule_accessors.hpp>
#include <rexlib/core/dispatch/rules/shape_rule_descriptor_of.hpp>
#include <rexlib/core/dispatch/rules/type_rule_accessors.hpp>

#include <utility>

namespace rexlib
{

template <typename ShapeRule, typename TypeRule, typename Parameters>
template <typename... Args, typename>
inline
rule_contract<ShapeRule, TypeRule, Parameters>
::rule_contract(Args&&... args)
	: m_parameters(std::forward<Args>(args)...)
{
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
const Parameters&
rule_contract<ShapeRule, TypeRule, Parameters>
::get_parameters() const noexcept
{
	return m_parameters;
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
void rule_contract<ShapeRule, TypeRule, Parameters>
::check_parameters(const operation_descriptor &descriptor) const
{
	using shape_accessors = shape_rule_accessors<ShapeRule>;
	using type_accessors = type_rule_accessors<TypeRule>;

	const auto extents =
		shape_accessors::get_operation_extents(m_parameters);
	get_shape_interpreter().check_instance(
		descriptor,
		shape_accessors::get_named_axes(m_parameters),
		make_span(extents.data(), extents.size())
	);

	const auto pivots = type_accessors::get_operation_pivots(m_parameters);
	get_type_interpreter().check_instance(
		descriptor,
		make_span(pivots.data(), pivots.size())
	);
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
void rule_contract<ShapeRule, TypeRule, Parameters>::deduce(
	const operation_descriptor &descriptor,
	span<operand_specification> canonical_outputs,
	span<const operand_specification> inputs
) const
{
	using shape_accessors = shape_rule_accessors<ShapeRule>;
	using type_accessors = type_rule_accessors<TypeRule>;

	const auto extents =
		shape_accessors::get_operation_extents(m_parameters);
	get_shape_interpreter().deduce(
		descriptor,
		shape_accessors::get_named_axes(m_parameters),
		shape_accessors::get_keep_dimensions(m_parameters),
		make_span(extents.data(), extents.size()),
		canonical_outputs,
		inputs
	);

	const auto pivots = type_accessors::get_operation_pivots(m_parameters);
	get_type_interpreter().deduce(
		descriptor,
		make_span(pivots.data(), pivots.size()),
		canonical_outputs,
		inputs
	);
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
void rule_contract<ShapeRule, TypeRule, Parameters>::accept(
	const operation_descriptor &descriptor,
	span<const operand_specification> user_outputs,
	span<const operand_specification> /*canonical_outputs*/,
	span<const operand_specification> inputs
) const
{
	using shape_accessors = shape_rule_accessors<ShapeRule>;
	using type_accessors = type_rule_accessors<TypeRule>;

	const auto extents =
		shape_accessors::get_operation_extents(m_parameters);
	get_shape_interpreter().accept(
		descriptor,
		shape_accessors::get_named_axes(m_parameters),
		shape_accessors::get_keep_dimensions(m_parameters),
		make_span(extents.data(), extents.size()),
		user_outputs,
		inputs
	);

	const auto pivots = type_accessors::get_operation_pivots(m_parameters);
	get_type_interpreter().accept(
		descriptor,
		make_span(pivots.data(), pivots.size()),
		user_outputs,
		inputs
	);
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
shape_rule_interpreter
rule_contract<ShapeRule, TypeRule, Parameters>
::get_shape_interpreter() noexcept
{
	return shape_rule_interpreter(shape_rule_descriptor_of<ShapeRule>::get());
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
type_rule_interpreter
rule_contract<ShapeRule, TypeRule, Parameters>
::get_type_interpreter() noexcept
{
	return type_rule_interpreter(
		pivot_descriptor_table<typename TypeRule::pivot_list>::get(),
		slot_descriptor_table<typename TypeRule::output_slot_list>::get(),
		slot_descriptor_table<typename TypeRule::input_slot_list>::get()
	);
}

} // namespace rexlib
