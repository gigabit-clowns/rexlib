// SPDX-License-Identifier: GPL-3.0-only

#include "kaiser_bessel_window.hpp"

#include "bessel_i0.hpp"

#include <rexlib/core/platform/assert.hpp>

#include <algorithm>
#include <cmath>

namespace rexlib
{
namespace cpu
{

inline
kaiser_bessel_window::kaiser_bessel_window(const em::gridding_window &window)
	: m_half_width(window.get_half_width())
	, m_tap_count(static_cast<std::size_t>(std::ceil(2.0 * m_half_width)))
{
	REXLIB_ASSERT(window.get_profile() == em::gridding_profile::kaiser_bessel);

	const auto shape = window.get_shape();
	const auto normalization = 1.0 / bessel_i0(shape);
	const auto count =
		static_cast<std::size_t>(std::ceil(m_half_width * samples_per_voxel));

	// The last entry is the window at the half-width itself, which the
	// stretch before it interpolates towards.
	m_table.resize(count + 1);
	for (std::size_t i = 0; i <= count; ++i)
	{
		const auto ratio = std::min(
			static_cast<double>(i) / (m_half_width * samples_per_voxel),
			1.0
		);
		m_table[i] =
			bessel_i0(shape * std::sqrt(1.0 - ratio * ratio)) * normalization;
	}
}

inline
std::size_t kaiser_bessel_window::get_tap_count() const noexcept
{
	return m_tap_count;
}

inline
double kaiser_bessel_window::get_half_width() const noexcept
{
	return m_half_width;
}

inline
double kaiser_bessel_window::evaluate(double distance) const noexcept
{
	if (!(std::abs(distance) < m_half_width))
	{
		return 0.0;
	}

	const auto position = std::abs(distance) * samples_per_voxel;
	const auto below = std::floor(position);
	const auto fraction = position - below;
	const auto index = static_cast<std::size_t>(below);
	return m_table[index] + fraction * (m_table[index + 1] - m_table[index]);
}

template <typename T>
inline
std::ptrdiff_t kaiser_bessel_window::place(
	double coordinate,
	T *weights
) const noexcept
{
	const auto first =
		static_cast<std::ptrdiff_t>(std::floor(coordinate - m_half_width)) + 1;
	for (std::size_t tap = 0; tap < m_tap_count; ++tap)
	{
		const auto voxel = first + static_cast<std::ptrdiff_t>(tap);
		weights[tap] =
			static_cast<T>(evaluate(static_cast<double>(voxel) - coordinate));
	}
	return first;
}

} // namespace cpu
} // namespace rexlib
