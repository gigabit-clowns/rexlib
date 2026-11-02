// SPDX-License-Identifier: GPL-3.0-only

#include "row_insertion_kernel.hpp"

#include <algorithm>
#include <cmath>

namespace rexlib
{
namespace cpu
{

template <
	typename T,
	typename Window,
	typename VolumePhase,
	typename SampleWeight
>
inline
row_insertion_kernel<T, Window, VolumePhase, SampleWeight>::
row_insertion_kernel(
	const Window &window,
	const VolumePhase &phase,
	std::size_t volume_extent,
	const operand_core &numerator_core,
	const operand_core &denominator_core
)
	: m_window(&window)
	, m_phase(&phase)
	, m_extent(static_cast<std::ptrdiff_t>(volume_extent))
	, m_half_extent(static_cast<std::ptrdiff_t>(volume_extent / 2))
	, m_tap_count(window.get_tap_count())
	, m_numerator_core(numerator_core)
	, m_denominator_core(denominator_core)
	, m_numerator(nullptr)
	, m_denominator(nullptr)
	, m_z(0, 0)
	, m_y(0, 0)
	, m_reach({{ 0, 0, 0 }}, {{ 0, 0, 0 }})
	, m_first_x(chunk_size)
	, m_first_y(chunk_size)
	, m_first_z(chunk_size)
	, m_weights_x(chunk_size * m_tap_count)
	, m_weights_y(chunk_size * m_tap_count)
	, m_weights_z(chunk_size * m_tap_count)
	, m_numerator_weights(chunk_size)
	, m_denominator_weights(chunk_size)
	, m_values(chunk_size)
	, m_counts(chunk_size)
	, m_active(chunk_size)
{
}

template <
	typename T,
	typename Window,
	typename VolumePhase,
	typename SampleWeight
>
inline
void row_insertion_kernel<T, Window, VolumePhase, SampleWeight>::bind(
	complex_type *numerator,
	T *denominator
) noexcept
{
	m_numerator = numerator;
	m_denominator = denominator;
}

template <
	typename T,
	typename Window,
	typename VolumePhase,
	typename SampleWeight
>
inline
void row_insertion_kernel<T, Window, VolumePhase, SampleWeight>::set_box(
	const frequency_interval &z,
	const frequency_interval &y
) noexcept
{
	m_z = z;
	m_y = y;

	const auto half_width = 0.5 * static_cast<double>(m_tap_count);
	m_reach = frequency_bounds(
		{{
			0.0,
			static_cast<double>(y.get_lower()),
			static_cast<double>(z.get_lower())
		}},
		{{
			static_cast<double>(m_half_extent),
			static_cast<double>(y.get_upper()),
			static_cast<double>(z.get_upper())
		}}
	).widened(half_width);
}

template <
	typename T,
	typename Window,
	typename VolumePhase,
	typename SampleWeight
>
inline
const frequency_bounds&
row_insertion_kernel<T, Window, VolumePhase, SampleWeight>::get_reach()
	const noexcept
{
	return m_reach;
}

template <
	typename T,
	typename Window,
	typename VolumePhase,
	typename SampleWeight
>
inline
void row_insertion_kernel<T, Window, VolumePhase, SampleWeight>::insert(
	const virtual_row<T> &row,
	const slice_state_type &state,
	std::ptrdiff_t begin,
	std::ptrdiff_t end
)
{
	for (auto k = begin; k < end; k += static_cast<std::ptrdiff_t>(chunk_size))
	{
		const auto count = static_cast<std::size_t>(
			std::min(end - k, static_cast<std::ptrdiff_t>(chunk_size))
		);
		prepare(row, state, k, count);
		scatter(count);
	}
}

template <
	typename T,
	typename Window,
	typename VolumePhase,
	typename SampleWeight
>
inline
void row_insertion_kernel<T, Window, VolumePhase, SampleWeight>::prepare(
	const virtual_row<T> &row,
	const slice_state_type &state,
	std::ptrdiff_t begin,
	std::size_t count
)
{
	SampleWeight::fill(
		state,
		row.get_stored_row(),
		row.get_k_y(),
		begin,
		count,
		m_numerator_weights.data(),
		m_denominator_weights.data()
	);

	const auto &base = row.get_base();
	const auto &step = row.get_step();
	const auto k_y = row.get_k_y();
	const auto squared_k_y = k_y * k_y;
	const auto last_shell = row.get_last_shell();
	const auto *samples = row.get_samples();
	const auto sample_stride = row.get_sample_stride();
	const auto *x_phases = row.get_x_phases();
	const auto y_phase = row.get_y_phase();
	const auto *radial = row.get_radial_weights();
	const auto radial_stride = row.get_radial_stride();
	const bool negated = row.is_negated();

	for (std::size_t i = 0; i < count; ++i)
	{
		const auto k = begin + static_cast<std::ptrdiff_t>(i);
		const auto k_x = static_cast<double>(k);

		m_first_x[i] = m_window->place(
			base[0] + k_x * step[0], &m_weights_x[i * m_tap_count]
		);
		m_first_y[i] = m_window->place(
			base[1] + k_x * step[1], &m_weights_y[i * m_tap_count]
		);
		m_first_z[i] = m_window->place(
			base[2] + k_x * step[2], &m_weights_z[i * m_tap_count]
		);

		const auto shell = static_cast<std::ptrdiff_t>(
			std::floor(std::sqrt(k_x * k_x + squared_k_y) + 0.5)
		);
		const bool within = shell <= last_shell;
		const auto radial_weight =
			within ? radial[shell * radial_stride] : T(0);
		const bool active = within && radial_weight != T(0);
		m_active[i] = active ? 1 : 0;

		auto value = samples[k * sample_stride] * x_phases[k] * y_phase;
		if (negated)
		{
			value = std::conj(value);
		}
		m_values[i] = active
			? value * (radial_weight * m_numerator_weights[i])
			: complex_type(0);
		m_counts[i] = active
			? radial_weight * m_denominator_weights[i]
			: T(0);
	}
}

template <
	typename T,
	typename Window,
	typename VolumePhase,
	typename SampleWeight
>
inline
void row_insertion_kernel<T, Window, VolumePhase, SampleWeight>::scatter(
	std::size_t count
) noexcept
{
	const auto numerator_z = m_numerator_core.get_stride(0);
	const auto numerator_y = m_numerator_core.get_stride(1);
	const auto numerator_x = m_numerator_core.get_stride(2);
	const auto denominator_z = m_denominator_core.get_stride(0);
	const auto denominator_y = m_denominator_core.get_stride(1);
	const auto denominator_x = m_denominator_core.get_stride(2);

	for (std::size_t i = 0; i < count; ++i)
	{
		if (!m_active[i])
		{
			continue;
		}

		const auto value = m_values[i];
		const auto weight = m_counts[i];
		const auto *weights_x = &m_weights_x[i * m_tap_count];
		const auto *weights_y = &m_weights_y[i * m_tap_count];
		const auto *weights_z = &m_weights_z[i * m_tap_count];

		for (std::size_t tz = 0; tz < m_tap_count; ++tz)
		{
			const auto z = m_first_z[i] + static_cast<std::ptrdiff_t>(tz);
			if (z < m_z.get_lower() || z > m_z.get_upper())
			{
				continue;
			}
			const auto stored_z = stored_index(z);

			for (std::size_t ty = 0; ty < m_tap_count; ++ty)
			{
				const auto y = m_first_y[i] + static_cast<std::ptrdiff_t>(ty);
				if (y < m_y.get_lower() || y > m_y.get_upper())
				{
					continue;
				}
				const auto stored_y = stored_index(y);
				const auto weight_zy = weights_z[tz] * weights_y[ty];
				const auto numerator_row = m_numerator +
					stored_z * numerator_z + stored_y * numerator_y;
				const auto denominator_row = m_denominator +
					stored_z * denominator_z + stored_y * denominator_y;

				for (std::size_t tx = 0; tx < m_tap_count; ++tx)
				{
					const auto x =
						m_first_x[i] + static_cast<std::ptrdiff_t>(tx);
					if (x < 0 || x > m_half_extent)
					{
						continue;
					}
					const auto w = weight_zy * weights_x[tx];
					numerator_row[x * numerator_x] +=
						m_phase->apply(value * w, x, stored_y, stored_z);
					denominator_row[x * denominator_x] += weight * w;
				}
			}
		}
	}
}

template <
	typename T,
	typename Window,
	typename VolumePhase,
	typename SampleWeight
>
inline
std::ptrdiff_t
row_insertion_kernel<T, Window, VolumePhase, SampleWeight>::stored_index(
	std::ptrdiff_t frequency
) const noexcept
{
	return frequency < 0 ? frequency + m_extent : frequency;
}

} // namespace cpu
} // namespace rexlib
