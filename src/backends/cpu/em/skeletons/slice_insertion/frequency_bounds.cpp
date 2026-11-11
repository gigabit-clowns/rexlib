// SPDX-License-Identifier: GPL-3.0-only

#include "frequency_bounds.hpp"

namespace rexlib
{
namespace cpu
{

frequency_bounds::frequency_bounds(
	const volume_frequency &lower,
	const volume_frequency &upper
) noexcept
	: m_lower(lower)
	, m_upper(upper)
{
}

double frequency_bounds::get_lower(std::size_t axis) const noexcept
{
	return m_lower[axis];
}

double frequency_bounds::get_upper(std::size_t axis) const noexcept
{
	return m_upper[axis];
}

frequency_bounds frequency_bounds::widened(double margin) const noexcept
{
	return frequency_bounds(
		{{ m_lower[0] - margin, m_lower[1] - margin, m_lower[2] - margin }},
		{{ m_upper[0] + margin, m_upper[1] + margin, m_upper[2] + margin }}
	);
}

} // namespace cpu
} // namespace rexlib
