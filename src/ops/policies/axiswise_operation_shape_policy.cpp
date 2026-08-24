// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/policies/axiswise_operation_shape_policy.hpp>

#include "shape_deduction.hpp"

#include <utility>

namespace rexlib
{
namespace ops
{

axiswise_operation_shape_policy::axiswise_operation_shape_policy(
	axis_list axes
)
	: m_axes(make_sorted_axis_list(std::move(axes)))
{
}

span<const std::size_t>
axiswise_operation_shape_policy::get_axes() const noexcept
{
	return make_span(m_axes.data(), m_axes.size());
}

void axiswise_operation_shape_policy::deduce(
	const operation_descriptor &descriptor,
	span<shape_type> canonical_output_shapes,
	span<const shape_type> input_shapes
) const
{
	auto shape = broadcast_input_shapes(descriptor, input_shapes);

	// The rank is only known here, so this is the first place the axes can
	// be checked against it.
	check_axes_within_rank(descriptor, get_axes(), shape.size());

	// The shape comes out as it went in, the axes saying what the operation
	// does rather than what it produces.
	assign_output_shapes(canonical_output_shapes, std::move(shape));
}

} // namespace ops
} // namespace rexlib
