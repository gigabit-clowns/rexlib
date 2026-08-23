// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/policies/reduction_operation_shape_policy.hpp>

#include "shape_deduction.hpp"

#include <iterator>
#include <utility>

namespace rex
{
namespace ops
{

reduction_operation_shape_policy::reduction_operation_shape_policy(
	axis_list_type axes,
	bool keep_dimensions
)
	: m_axes(make_sorted_axis_list(std::move(axes)))
	, m_keep_dimensions(keep_dimensions)
{
}

span<const std::size_t>
reduction_operation_shape_policy::get_axes() const noexcept
{
	return make_span(m_axes.data(), m_axes.size());
}

bool reduction_operation_shape_policy::get_keep_dimensions() const noexcept
{
	return m_keep_dimensions;
}

void reduction_operation_shape_policy::deduce(
	const operation_descriptor &descriptor,
	span<shape_type> canonical_output_shapes,
	span<const shape_type> input_shapes
) const
{
	auto shape = broadcast_input_shapes(descriptor, input_shapes);

	// The rank is only known here, so this is the first place the axes can
	// be checked against it.
	check_axes_within_rank(descriptor, get_axes(), shape.size());

	if (m_keep_dimensions)
	{
		for (const auto axis : m_axes)
		{
			shape[axis] = 1;
		}
	}
	else
	{
		// Back to front, so that the indices not yet used stay valid.
		// The constructor sorted them ascending for exactly this.
		for (auto ite = m_axes.rbegin(); ite != m_axes.rend(); ++ite)
		{
			shape.erase(std::next(shape.begin(), *ite));
		}
	}

	assign_output_shapes(canonical_output_shapes, std::move(shape));
}

} // namespace ops
} // namespace rex
