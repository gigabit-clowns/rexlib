// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/operation/shape_rule/shape_rule_interpreter.hpp>

#include "shape_rule_engine.hpp"

namespace rexlib
{

shape_rule_interpreter::shape_rule_interpreter(
	const shape_rule_descriptor &rule
) noexcept
	: m_rule(&rule)
{
}

void shape_rule_interpreter::check_instance(
	const operation_descriptor &descriptor,
	span<const std::size_t> named_axes,
	span<const std::size_t> operation_extents
) const
{
	check_shape_rule_instance(
		descriptor,
		*m_rule,
		named_axes,
		operation_extents
	);
}

void shape_rule_interpreter::deduce(
	const operation_descriptor &descriptor,
	span<const std::size_t> named_axes,
	bool keep_dimensions,
	span<const std::size_t> operation_extents,
	span<operand_specification> canonical_outputs,
	span<const operand_specification> inputs
) const
{
	const auto resolution = resolve_shape_rule(
		descriptor,
		*m_rule,
		named_axes,
		operation_extents,
		canonical_outputs.size(),
		inputs
	);

	for (std::size_t o = 0; o < canonical_outputs.size(); ++o)
	{
		canonical_outputs[o].set_shape(
			make_output_shape(
				*m_rule,
				resolution,
				named_axes,
				keep_dimensions,
				o
			)
		);
	}
}

void shape_rule_interpreter::accept(
	const operation_descriptor &descriptor,
	span<const std::size_t> named_axes,
	bool keep_dimensions,
	span<const std::size_t> operation_extents,
	span<const operand_specification> user_outputs,
	span<const operand_specification> inputs
) const
{
	const auto resolution = resolve_shape_rule(
		descriptor,
		*m_rule,
		named_axes,
		operation_extents,
		user_outputs.size(),
		inputs
	);

	check_output_shapes(
		descriptor,
		*m_rule,
		resolution,
		named_axes,
		keep_dimensions,
		user_outputs
	);
}

} // namespace rexlib
