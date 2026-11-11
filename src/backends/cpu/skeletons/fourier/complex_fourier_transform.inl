// SPDX-License-Identifier: GPL-3.0-only

#include "complex_fourier_transform.hpp"

#include <backends/cpu/skeletons/fourier/fourier_conversion.hpp>

namespace rexlib
{
namespace cpu
{

template <typename Fft, ops::fourier_direction Direction>
inline
complex_fourier_transform<Fft, Direction>::complex_fourier_transform(
	ops::fourier_normalization normalization
) noexcept
	: m_normalization(normalization)
{
}

template <typename Fft, ops::fourier_direction Direction>
inline
fourier_transform_kind
complex_fourier_transform<Fft, Direction>::get_kind() noexcept
{
	return fourier_transform_kind::complex_to_complex;
}

template <typename Fft, ops::fourier_direction Direction>
template <typename T>
inline
void complex_fourier_transform<Fft, Direction>::operator()(
	const fourier_layout_plan &plan,
	std::complex<T> *output,
	const std::complex<T> *input,
	const loop_schedule &schedule
) const
{
	if (plan.get_axes().empty())
	{
		run_fourier_conversion(plan, output, input, schedule);
	}
	else
	{
		Fft::complex_to_complex(
			plan,
			Direction,
			m_normalization,
			output,
			input,
			schedule.get_concurrency()
		);
	}
}

template <typename Fft, ops::fourier_direction Direction>
template <typename T>
inline
void complex_fourier_transform<Fft, Direction>::operator()(
	const fourier_layout_plan &plan,
	std::complex<T> *output,
	const T *input,
	const loop_schedule &schedule
) const
{
	run_fourier_conversion(plan, output, input, schedule);

	if (!plan.get_axes().empty())
	{
		Fft::in_place_complex(
			plan,
			Direction,
			m_normalization,
			output,
			schedule.get_concurrency()
		);
	}
}

} // namespace cpu
} // namespace rexlib
