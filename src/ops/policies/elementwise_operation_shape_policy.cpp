// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/policies/elementwise_operation_shape_policy.hpp>

#include "shape_deduction.hpp"

#include <rexlib/core/layout/broadcast.hpp>
#include <rexlib/core/platform/assert.hpp>

#include <sstream>
#include <stdexcept>

namespace rexlib
{
namespace ops
{

void elementwise_operation_shape_policy::deduce(
	const operation_descriptor &descriptor,
	span<shape_type> canonical_output_shapes,
	span<const shape_type> input_shapes
) const
{
	// Unlike the rest of the family this admits having nothing to read a
	// shape off, which is how an operation that only writes, such as a fill,
	// leaves its outputs the shape they came with.
	assign_output_shapes(
		canonical_output_shapes,
		input_shapes.empty()
			? shape_type()
			: broadcast_input_shapes(descriptor, input_shapes)
	);
}

void elementwise_operation_shape_policy::accept(
	const operation_descriptor &descriptor,
	span<const shape_type> user_output_shapes,
	span<const shape_type> canonical_output_shapes,
	span<const shape_type> /*input_shapes*/
) const
{
	REXLIB_ASSERT(
		user_output_shapes.size() == canonical_output_shapes.size()
	);

	if (user_output_shapes.empty())
	{
		return;
	}

	const auto &reference_shape = user_output_shapes[0];
	for (std::size_t i = 1; i < user_output_shapes.size(); ++i)
	{
		if (user_output_shapes[i] != reference_shape)
		{
			std::ostringstream oss;
			oss << descriptor << ": output operand "
				<< describe_operand(descriptor, i, true)
				<< " does not have the same shape as output operand "
				<< describe_operand(descriptor, 0, true)
				<< ", which an elementwise operation requires.";
			throw std::invalid_argument(oss.str());
		}
	}

	const auto valid = is_broadcastable_to(
		make_span(canonical_output_shapes[0]),
		make_span(reference_shape)
	);
	if (!valid)
	{
		std::ostringstream oss;
		oss << descriptor << ": output operand "
			<< describe_operand(descriptor, 0, true)
			<< " has a shape the inputs cannot be broadcast to.";
		throw std::invalid_argument(oss.str());
	}
}

const elementwise_operation_shape_policy&
elementwise_operation_shape_policy::get() noexcept
{
	static const elementwise_operation_shape_policy instance;
	return instance;
}

} // namespace ops
} // namespace rexlib
