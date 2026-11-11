// SPDX-License-Identifier: GPL-3.0-only

#include "complex_to_real_fourier_transform.hpp"

namespace rexlib
{
namespace cpu
{

template <typename Fft>
inline
complex_to_real_fourier_transform<Fft>::complex_to_real_fourier_transform(
	ops::fourier_normalization normalization
) noexcept
	: m_normalization(normalization)
{
}

template <typename Fft>
inline
fourier_transform_kind
complex_to_real_fourier_transform<Fft>::get_kind() noexcept
{
	return fourier_transform_kind::complex_to_real;
}

template <typename Fft>
template <typename T>
inline
void complex_to_real_fourier_transform<Fft>::operator()(
	const fourier_layout_plan &plan,
	T *output,
	const std::complex<T> *input,
	const loop_schedule &schedule
) const
{
	Fft::complex_to_real(
		plan,
		ops::fourier_direction::inverse,
		m_normalization,
		output,
		input,
		schedule.get_concurrency()
	);
}

} // namespace cpu
} // namespace rexlib
