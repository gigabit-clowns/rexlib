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
{
	for (auto &weights : m_window_weights)
	{
		weights.resize(chunk_size * m_tap_count);
	}
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
	chunk_type chunk;
	const auto step = static_cast<std::ptrdiff_t>(chunk_size);
	for (auto k = begin; k < end; k += step)
	{
		const auto count =
			static_cast<std::int32_t>(std::min(end - k, step));
		prepare(row, state, k, count, chunk);
		scatter(count, chunk);
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
	std::int32_t count,
	chunk_type &chunk
)
{
	SampleWeight::fill(
		state,
		row.get_stored_row(),
		row.get_k_y(),
		begin,
		static_cast<std::size_t>(count),
		chunk.get_numerator_weights().data(),
		chunk.get_denominator_weights().data()
	);

	const auto first = static_cast<T>(begin);
	for (std::size_t axis = 0; axis < 3; ++axis)
	{
		const auto base = static_cast<T>(row.get_base()[axis]);
		const auto step = static_cast<T>(row.get_step()[axis]);
		auto *coordinates = chunk.get_coordinates(axis).data();
		for (std::int32_t i = 0; i < count; ++i)
		{
			coordinates[i] = base + (first + static_cast<T>(i)) * step;
		}
		m_window->place(
			coordinates,
			count,
			chunk.get_first_voxels(axis).data(),
			m_window_weights[axis].data(),
			chunk_size
		);
	}

	fill_radial_weights(row, begin, count, chunk);
	fill_values(row, begin, count, chunk);
}

template <
	typename T,
	typename Window,
	typename VolumePhase,
	typename SampleWeight
>
inline
void row_insertion_kernel<T, Window, VolumePhase, SampleWeight>::
fill_radial_weights(
	const virtual_row<T> &row,
	std::ptrdiff_t begin,
	std::int32_t count,
	chunk_type &chunk
) const noexcept
{
	// The shell round(|k|) of the samples of a row never decreases along
	// it, and is found without a square root: (s + 1/2)^2 <= |k|^2 moves to
	// the next shell, that is 4 s^2 + 4 s + 1 <= 4 |k|^2 in integers.
	const auto k_y = static_cast<std::int64_t>(row.get_k_y());
	const auto last_shell = static_cast<std::int64_t>(row.get_last_shell());
	const auto *radial = row.get_radial_weights();
	const auto radial_stride = row.get_radial_stride();
	auto &radial_weights = chunk.get_radial_weights();

	const auto first = static_cast<std::int64_t>(begin);
	auto shell = static_cast<std::int64_t>(std::floor(
		std::sqrt(static_cast<double>(first * first + k_y * k_y)) + 0.5
	));
	for (std::int32_t i = 0; i < count; ++i)
	{
		const auto k_x = first + i;
		const auto quadruple_norm = 4 * (k_x * k_x + k_y * k_y);
		while (4 * shell * shell + 4 * shell + 1 <= quadruple_norm)
		{
			++shell;
		}
		radial_weights[static_cast<std::size_t>(i)] = shell <= last_shell
			? radial[static_cast<std::ptrdiff_t>(shell) * radial_stride]
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
void row_insertion_kernel<T, Window, VolumePhase, SampleWeight>::fill_values(
	const virtual_row<T> &row,
	std::ptrdiff_t begin,
	std::int32_t count,
	chunk_type &chunk
) const noexcept
{
	// std::complex holds its parts as an array of two, and a product of
	// std::complex calls into the library for NaN operands, so the
	// products are written out on the parts. A sample whose radial weight
	// is 0 is computed too, NaN or not, and skipped by the second pass.
	const auto *samples = reinterpret_cast<const T*>(
		row.get_samples() + begin * row.get_sample_stride()
	);
	const auto sample_stride = 2 * row.get_sample_stride();
	const auto *x_phases = reinterpret_cast<const T*>(
		row.get_x_phases() + begin
	);
	const auto y_real = row.get_y_phase().real();
	const auto y_imaginary = row.get_y_phase().imag();
	const T sign = row.is_negated() ? T(-1) : T(1);
	const auto *radial_weights = chunk.get_radial_weights().data();
	const auto *numerator_weights = chunk.get_numerator_weights().data();
	const auto *denominator_weights = chunk.get_denominator_weights().data();
	auto *values_real = chunk.get_values_real().data();
	auto *values_imaginary = chunk.get_values_imaginary().data();
	auto *counts = chunk.get_counts().data();

	for (std::int32_t i = 0; i < count; ++i)
	{
		const auto sample_real = samples[i * sample_stride];
		const auto sample_imaginary = samples[i * sample_stride + 1];
		// NOLINTNEXTLINE(bugprone-implicit-widening-of-multiplication-result)
		const auto x_real = x_phases[2 * i];
		const auto x_imaginary = x_phases[2 * i + 1];

		const auto shifted_real =
			sample_real * x_real - sample_imaginary * x_imaginary;
		const auto shifted_imaginary =
			sample_real * x_imaginary + sample_imaginary * x_real;
		const auto value_real =
			shifted_real * y_real - shifted_imaginary * y_imaginary;
		const auto value_imaginary =
			shifted_real * y_imaginary + shifted_imaginary * y_real;

		const auto scale = radial_weights[i] * numerator_weights[i];
		values_real[i] = value_real * scale;
		values_imaginary[i] = sign * value_imaginary * scale;
		counts[i] = radial_weights[i] * denominator_weights[i];
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
	std::int32_t count,
	const chunk_type &chunk
) noexcept
{
	const auto numerator_z = m_numerator_core.get_stride(0);
	const auto numerator_y = m_numerator_core.get_stride(1);
	const auto numerator_x = m_numerator_core.get_stride(2);
	const auto denominator_z = m_denominator_core.get_stride(0);
	const auto denominator_y = m_denominator_core.get_stride(1);
	const auto denominator_x = m_denominator_core.get_stride(2);
	const auto &first_x = chunk.get_first_voxels(0);
	const auto &first_y = chunk.get_first_voxels(1);
	const auto &first_z = chunk.get_first_voxels(2);
	const auto &weights_x = m_window_weights[0];
	const auto &weights_y = m_window_weights[1];
	const auto &weights_z = m_window_weights[2];

	for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i)
	{
		if (chunk.get_radial_weights()[i] == T(0))
		{
			continue;
		}

		const complex_type value(
			chunk.get_values_real()[i],
			chunk.get_values_imaginary()[i]
		);
		const auto weight = chunk.get_counts()[i];
		for (std::size_t tz = 0; tz < m_tap_count; ++tz)
		{
			const auto z = static_cast<std::ptrdiff_t>(first_z[i]) +
				static_cast<std::ptrdiff_t>(tz);
			if (z < m_z.get_lower() || z > m_z.get_upper())
			{
				continue;
			}
			const auto stored_z = stored_index(z);
			const auto weight_z = weights_z[tz * chunk_size + i];

			for (std::size_t ty = 0; ty < m_tap_count; ++ty)
			{
				const auto y = static_cast<std::ptrdiff_t>(first_y[i]) +
					static_cast<std::ptrdiff_t>(ty);
				if (y < m_y.get_lower() || y > m_y.get_upper())
				{
					continue;
				}
				const auto stored_y = stored_index(y);
				const auto weight_zy =
					weight_z * weights_y[ty * chunk_size + i];
				const auto numerator_row = m_numerator +
					stored_z * numerator_z + stored_y * numerator_y;
				const auto denominator_row = m_denominator +
					stored_z * denominator_z + stored_y * denominator_y;

				for (std::size_t tx = 0; tx < m_tap_count; ++tx)
				{
					const auto x = static_cast<std::ptrdiff_t>(first_x[i]) +
						static_cast<std::ptrdiff_t>(tx);
					if (x < 0 || x > m_half_extent)
					{
						continue;
					}
					const auto w = weight_zy * weights_x[tx * chunk_size + i];
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
