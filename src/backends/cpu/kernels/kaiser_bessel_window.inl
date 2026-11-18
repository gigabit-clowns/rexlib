// SPDX-License-Identifier: GPL-3.0-only

#include "kaiser_bessel_window.hpp"

#include "bessel_i0.hpp"

#include <rexlib/core/platform/assert.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace rexlib
{
namespace cpu
{

template <typename T>
inline
kaiser_bessel_window<T>::kaiser_bessel_window(
	const em::gridding_window &window
)
	: m_half_width(static_cast<T>(window.get_half_width()))
	, m_tap_count(
		static_cast<std::size_t>(std::ceil(2.0 * window.get_half_width()))
	)
{
	REXLIB_ASSERT(window.get_profile() == em::gridding_profile::kaiser_bessel);

	const auto half_width = window.get_half_width();
	const auto shape = window.get_shape();
	const auto normalization = 1.0 / bessel_i0(shape);
	const auto count =
		static_cast<std::size_t>(std::ceil(half_width * samples_per_voxel));
	m_scale = static_cast<T>(static_cast<double>(count) / half_width);
	m_step_count = static_cast<std::int32_t>(count);

	// The entry i holds the window at i / count of the half-width, so the
	// entry count holds its value at the half-width itself; two zeros
	// follow, read for any distance from the half-width on.
	m_table.assign(count + 3, T(0));
	for (std::size_t i = 0; i <= count; ++i)
	{
		const auto ratio = static_cast<double>(i) / static_cast<double>(count);
		m_table[i] = static_cast<T>(
			bessel_i0(shape * std::sqrt(1.0 - ratio * ratio)) * normalization
		);
	}
}

template <typename T>
inline
std::size_t kaiser_bessel_window<T>::get_tap_count() const noexcept
{
	return m_tap_count;
}

template <typename T>
inline
T kaiser_bessel_window<T>::get_half_width() const noexcept
{
	return m_half_width;
}

template <typename T>
inline
T kaiser_bessel_window<T>::evaluate(T distance) const noexcept
{
	const auto position = std::abs(distance) * m_scale;
	const auto step = static_cast<std::int32_t>(position);
	const auto index = step < m_step_count ? step : m_step_count + 1;
	const auto fraction = position - static_cast<T>(index);
	const auto below = m_table[static_cast<std::size_t>(index)];
	return below +
		fraction * (m_table[static_cast<std::size_t>(index) + 1] - below);
}

template <typename T>
inline
void kaiser_bessel_window<T>::place(
	const T *coordinates,
	std::int32_t count,
	std::int32_t *first,
	T *weights,
	std::size_t weight_stride
) const noexcept
{
	const auto *table = m_table.data();

	// The lowest voxel above c - w: floor(c - w) + 1, the floor taken by
	// truncation and corrected below 0.
	for (std::int32_t i = 0; i < count; ++i)
	{
		const auto low = coordinates[i] - m_half_width;
		const auto truncated = static_cast<std::int32_t>(low);
		first[i] = truncated - (low < static_cast<T>(truncated) ? 1 : 0) + 1;
	}

	// A position at or past the step count, which is |d| >= w, reads the
	// two zeros past the end of the table: an integer select rather than
	// a floating point one. Each block is computed into the stack, where
	// the gather from the table cannot alias the store, then copied out.
	const auto past_end = m_step_count + 1;
	std::array<T, block_size> block{};
	for (std::size_t tap = 0; tap < m_tap_count; ++tap)
	{
		auto *tap_weights = weights + tap * weight_stride;
		const auto offset = static_cast<std::int32_t>(tap);
		for (std::int32_t start = 0; start < count; start += block_size)
		{
			const auto length = std::min(
				count - start,
				static_cast<std::int32_t>(block_size)
			);
			for (std::int32_t i = 0; i < length; ++i)
			{
				const auto j = start + i;
				const auto distance = std::abs(
					static_cast<T>(first[j] + offset) - coordinates[j]
				);
				const auto position = distance * m_scale;
				const auto step = static_cast<std::int32_t>(position);
				const auto index = step < m_step_count ? step : past_end;
				const auto fraction = position - static_cast<T>(index);
				const auto below = table[index];
				block[static_cast<std::size_t>(i)] =
					below + fraction * (table[index + 1] - below);
			}
			std::copy(
				block.begin(),
				block.begin() + length,
				tap_weights + start
			);
		}
	}
}

} // namespace cpu
} // namespace rexlib
