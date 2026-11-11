// SPDX-License-Identifier: GPL-3.0-only

#include "real_to_complex_fourier_transform.hpp"

namespace rexlib
{
namespace cpu
{

template <typename Fft>
inline
real_to_complex_fourier_transform<Fft>::real_to_complex_fourier_transform(
	ops::fourier_normalization normalization
) noexcept
	: m_normalization(normalization)
{
}

template <typename Fft>
inline
fourier_transform_kind
real_to_complex_fourier_transform<Fft>::get_kind() noexcept
{
	return fourier_transform_kind::real_to_complex;
}

template <typename Fft>
template <typename T>
inline
void real_to_complex_fourier_transform<Fft>::operator()(
	const fourier_layout_plan &plan,
	std::complex<T> *output,
	const T *input,
	const loop_schedule &schedule
) const
{
	Fft::real_to_complex(
		plan,
		ops::fourier_direction::forward,
		m_normalization,
		output,
		input,
		schedule.get_concurrency()
	);
}

} // namespace cpu
} // namespace rexlib
