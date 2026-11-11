// SPDX-License-Identifier: GPL-3.0-only

#include "contrast_transfer_function_2d.hpp"

#include <cmath>

namespace rexlib
{
namespace cpu
{

template <typename T>
inline
contrast_transfer_function_2d<T>::contrast_transfer_function_2d() noexcept
	: m_axial(0)
	, m_diagonal(0)
{
}

template <typename T>
inline
contrast_transfer_function_2d<T>::contrast_transfer_function_2d(
	const ctf_phase_coefficients &coefficients,
	double defocus_u,
	double defocus_v,
	double astigmatism_angle,
	double phase_shift
) noexcept
	: m_isotropic(
		coefficients,
		0.5 * (defocus_u + defocus_v),
		phase_shift
	)
{
	// pi L (D(t) - D_mean) s^2 = axial (k_x^2 - k_y^2) + diagonal k_x k_y,
	// from cos 2t = (k_x^2 - k_y^2) / |k|^2 and sin 2t = 2 k_x k_y / |k|^2.
	const auto half_difference =
		coefficients.get_defocus_scale() * 0.5 * (defocus_u - defocus_v);
	m_axial = static_cast<T>(
		half_difference * std::cos(2.0 * astigmatism_angle)
	);
	m_diagonal = static_cast<T>(
		2.0 * half_difference * std::sin(2.0 * astigmatism_angle)
	);
}

template <typename T>
inline
T contrast_transfer_function_2d<T>::operator()(T k_x, T k_y) const noexcept
{
	return std::sin(compute_phase(k_x, k_y));
}

template <typename T>
inline
T contrast_transfer_function_2d<T>::compute_phase(
	T k_x,
	T k_y
) const noexcept
{
	const auto xx = k_x * k_x;
	const auto yy = k_y * k_y;
	return
		m_isotropic.compute_squared_phase(xx + yy) +
		m_axial * (xx - yy) +
		m_diagonal * (k_x * k_y);
}

} // namespace cpu
} // namespace rexlib
