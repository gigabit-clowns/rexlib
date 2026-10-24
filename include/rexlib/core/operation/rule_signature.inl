// SPDX-License-Identifier: GPL-3.0-only

#include "rule_signature.hpp"

#include <rexlib/core/operation/shape_rule/shape_rule_accessors.hpp>
#include <rexlib/core/operation/shape_rule/shape_rule_descriptor_of.hpp>
#include <rexlib/core/operation/type_rule/type_rule_accessors.hpp>
#include <rexlib/core/operation/type_rule/type_rule_descriptor_of.hpp>

#include <utility>

namespace rexlib
{

template <typename ShapeRule, typename TypeRule, typename Parameters>
template <typename... Args, typename>
inline
rule_signature<ShapeRule, TypeRule, Parameters>
::rule_signature(Args&&... args)
	: m_parameters(std::forward<Args>(args)...)
{
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
const Parameters&
rule_signature<ShapeRule, TypeRule, Parameters>
::get_parameters() const noexcept
{
	return m_parameters;
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
void rule_signature<ShapeRule, TypeRule, Parameters>
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

	const auto data_types =
		type_accessors::get_operation_data_types(m_parameters);
	get_type_interpreter().check_instance(
		descriptor,
		make_span(data_types.data(), data_types.size())
	);
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
void rule_signature<ShapeRule, TypeRule, Parameters>::deduce(
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

	const auto data_types =
		type_accessors::get_operation_data_types(m_parameters);
	get_type_interpreter().deduce(
		descriptor,
		make_span(data_types.data(), data_types.size()),
		canonical_outputs,
		inputs
	);
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
void rule_signature<ShapeRule, TypeRule, Parameters>::accept(
	const operation_descriptor &descriptor,
	span<const operand_specification> user_outputs,
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

	const auto data_types =
		type_accessors::get_operation_data_types(m_parameters);
	get_type_interpreter().accept(
		descriptor,
		make_span(data_types.data(), data_types.size()),
		user_outputs,
		inputs
	);
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
operation_arity
rule_signature<ShapeRule, TypeRule, Parameters>::get_arity() const noexcept
{
	return operation_arity(TypeRule::output_count, TypeRule::input_count);
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
shape_rule_interpreter
rule_signature<ShapeRule, TypeRule, Parameters>
::get_shape_interpreter() noexcept
{
	return shape_rule_interpreter(shape_rule_descriptor_of<ShapeRule>::get());
}

template <typename ShapeRule, typename TypeRule, typename Parameters>
inline
type_rule_interpreter
rule_signature<ShapeRule, TypeRule, Parameters>
::get_type_interpreter() noexcept
{
	return type_rule_interpreter(type_rule_descriptor_of<TypeRule>::get());
}

} // namespace rexlib
