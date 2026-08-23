// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/policies/inverse_real_fourier_transform_shape_policy.hpp>

#include "shape_deduction.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

namespace rex
{
namespace ops
{

const char* to_string(signal_parity parity) noexcept
{
	switch (parity)
	{
	case signal_parity::even: return "even";
	case signal_parity::odd: return "odd";
	default: return "";
	}
}

std::ostream& operator<<(std::ostream &os, signal_parity parity)
{
	return os << to_string(parity);
}

std::size_t compute_real_signal_extent(
	std::size_t stored,
	signal_parity parity
) noexcept
{
	// The halved axis holds n/2 + 1 coefficients, which inverts to one of
	// two extents depending on how the rounding went.
	return parity == signal_parity::even
		? 2*(stored - 1)
		: 2*stored - 1;
}

signal_parity get_signal_parity(std::size_t extent) noexcept
{
	return (extent % 2) == 0 ? signal_parity::even : signal_parity::odd;
}

inverse_real_fourier_transform_shape_policy
::inverse_real_fourier_transform_shape_policy(
	axis_list axes,
	signal_parity parity
)
	: m_axes(make_sorted_axis_list(std::move(axes)))
	, m_parity(parity)
{
	if (m_axes.empty())
	{
		throw std::invalid_argument(
			"inverse_real_fourier_transform_shape_policy: at least one "
			"axis must be transformed, the restored one being the last of "
			"them."
		);
	}
}

span<const std::size_t>
inverse_real_fourier_transform_shape_policy::get_axes() const noexcept
{
	return make_span(m_axes.data(), m_axes.size());
}

signal_parity
inverse_real_fourier_transform_shape_policy::get_parity() const noexcept
{
	return m_parity;
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

	// Only the halved axis is restored. The others were never reduced, so
	// they already have the extent the samples had.
	const auto restored = m_axes.back();
	const auto stored = shape[restored];
	if (stored == 0)
	{
		std::ostringstream oss;
		oss << descriptor << ": axis " << restored
			<< " holds no coefficients, so there is no signal to restore.";
		throw std::invalid_argument(oss.str());
	}

	shape[restored] = compute_real_signal_extent(stored, m_parity);

	assign_output_shapes(canonical_output_shapes, std::move(shape));
}

} // namespace ops
} // namespace rex
