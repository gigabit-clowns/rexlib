// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/policies/real_fourier_transform_shape_policy.hpp>

#include "shape_deduction.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

namespace rex
{
namespace ops
{

real_fourier_transform_shape_policy::real_fourier_transform_shape_policy(
	axis_list axes
)
	: m_axes(make_sorted_axis_list(std::move(axes)))
{
	if (m_axes.empty())
	{
		throw std::invalid_argument(
			"real_fourier_transform_shape_policy: at least one axis must "
			"be transformed, the halved one being the last of them."
		);
	}
}

span<const std::size_t>
real_fourier_transform_shape_policy::get_axes() const noexcept
{
	return make_span(m_axes.data(), m_axes.size());
}

void real_fourier_transform_shape_policy::deduce(
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

	// The conjugate symmetry relates a coefficient to the one at the
	// negated index along every transformed axis at once, so it can be
	// exploited on one axis only. The last one is that axis; the rest keep
	// the extent they had.
	const auto halved = m_axes.back();
	shape[halved] = shape[halved]/2 + 1;

	assign_output_shapes(canonical_output_shapes, std::move(shape));
}

} // namespace ops
} // namespace rex
