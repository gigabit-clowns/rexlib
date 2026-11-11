// SPDX-License-Identifier: GPL-3.0-only

#include "contrast_transfer_function_1d.hpp"

#include <cmath>

namespace rexlib
{
namespace cpu
{

template <typename T>
inline
contrast_transfer_function_1d<T>::contrast_transfer_function_1d() noexcept
	: m_defocus(0)
	, m_aberration(0)
	, m_phase(0)
{
}

template <typename T>
inline
contrast_transfer_function_1d<T>::contrast_transfer_function_1d(
	const ctf_phase_coefficients &coefficients,
	double defocus,
	double phase_shift
) noexcept
	: m_defocus(static_cast<T>(coefficients.get_defocus_scale() * defocus))
	, m_aberration(static_cast<T>(coefficients.get_aberration()))
	, m_phase(static_cast<T>(
		phase_shift + coefficients.get_amplitude_phase()
	))
{
}

template <typename T>
inline
T contrast_transfer_function_1d<T>::operator()(T k) const noexcept
{
	return std::sin(compute_squared_phase(k * k));
}

template <typename T>
inline
T contrast_transfer_function_1d<T>::compute_squared_phase(
	T squared_frequency
) const noexcept
{
	return
		m_defocus * squared_frequency -
		m_aberration * (squared_frequency * squared_frequency) +
		m_phase;
}

} // namespace cpu
} // namespace rexlib
