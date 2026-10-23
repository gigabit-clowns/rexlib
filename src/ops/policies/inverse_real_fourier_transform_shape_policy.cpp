// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/policies/inverse_real_fourier_transform_shape_policy.hpp>

#include "shape_deduction.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace ops
{

inverse_real_fourier_transform_shape_policy
::inverse_real_fourier_transform_shape_policy(
	axis_list axes,
	std::size_t extent
)
	: m_axes(make_sorted_axis_list(std::move(axes)))
	, m_extent(extent)
{
	if (m_axes.empty())
	{
		throw std::invalid_argument(
			"inverse_real_fourier_transform_shape_policy: at least one "
			"axis must be transformed, the restored one being the last of "
			"them."
		);
	}

	if (m_extent == 0)
	{
		throw std::invalid_argument(
			"inverse_real_fourier_transform_shape_policy: the restored "
			"axis must hold at least one sample."
		);
	}
}

span<const std::size_t>
inverse_real_fourier_transform_shape_policy::get_axes() const noexcept
{
	return make_span(m_axes.data(), m_axes.size());
}

std::size_t
inverse_real_fourier_transform_shape_policy::get_extent() const noexcept
{
	return m_extent;
}

void inverse_real_fourier_transform_shape_policy::deduce(
	const operation_descriptor &descriptor,
	span<shape_type> canonical_output_shapes,
	span<const shape_type> input_shapes
) const
{
	if (input_shapes.size() != 1)
	{
		std::ostringstream oss;
		oss << descriptor << ": a Fourier transform takes exactly one "
			<< "input operand.";
		throw std::invalid_argument(oss.str());
	}

	shape_type shape = input_shapes[0];
	check_axes_within_rank(descriptor, get_axes(), shape.size());

	const auto restored = m_axes.back();
	const auto stored = shape[restored];
	const auto expected = m_extent / 2 + 1;
	if (stored != expected)
	{
		std::ostringstream oss;
		oss << descriptor << ": axis " << restored << " holds " << stored
			<< " coefficients, but a signal of " << m_extent
			<< " samples has " << expected << " of them.";
		throw std::invalid_argument(oss.str());
	}

	shape[restored] = m_extent;

	assign_output_shapes(canonical_output_shapes, std::move(shape));
}

} // namespace ops
} // namespace rexlib
