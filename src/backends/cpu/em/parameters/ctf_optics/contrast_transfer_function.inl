// SPDX-License-Identifier: GPL-3.0-only

#include "contrast_transfer_function.hpp"


#include <cmath>

#include <boost/math/constants/constants.hpp>

namespace rexlib
{
namespace cpu
{

template <typename T>
inline
contrast_transfer_function<T>::contrast_transfer_function() noexcept
	: m_radial(0)
	, m_axial(0)
	, m_diagonal(0)
	, m_aberration(0)
	, m_phase(0)
{
}

template <typename T>
inline
contrast_transfer_function<T>::contrast_transfer_function(
	const em::ctf_optics &optics,
	double pixel_size,
	std::size_t extent,
	double defocus_u,
	double defocus_v,
	double astigmatism_angle,
	double phase_shift
) noexcept
	: m_phase(static_cast<T>(
		phase_shift + std::asin(optics.get_amplitude_contrast())
	))
{
	using boost::math::double_constants::pi;

	const auto angstroms_per_millimetre = 1e7;
	const auto wavelength = optics.get_wavelength();
	const auto aberration =
		optics.get_spherical_aberration() * angstroms_per_millimetre;
	const auto frequency_scale =
		1.0 / (static_cast<double>(extent) * pixel_size);
	const auto squared_scale = frequency_scale * frequency_scale;
	const auto defocus_scale = pi * wavelength * squared_scale;
	const auto half_difference = 0.5 * (defocus_u - defocus_v);

	// pi L D(t) s^2 = radial |k|^2 + axial (k_x^2 - k_y^2) +
	// diagonal k_x k_y, from cos 2t = (k_x^2 - k_y^2) / |k|^2 and
	// sin 2t = 2 k_x k_y / |k|^2.
	m_radial = static_cast<T>(defocus_scale * 0.5 * (defocus_u + defocus_v));
	m_axial = static_cast<T>(
		defocus_scale * half_difference * std::cos(2.0 * astigmatism_angle)
	);
	m_diagonal = static_cast<T>(
		2.0 * defocus_scale * half_difference *
			std::sin(2.0 * astigmatism_angle)
	);
	m_aberration = static_cast<T>(
		0.5 * pi * aberration * wavelength * wavelength * wavelength *
			squared_scale * squared_scale
	);
}

template <typename T>
inline
T contrast_transfer_function<T>::operator()(T k_x, T k_y) const noexcept
{
	return std::sin(compute_phase(k_x, k_y));
}

template <typename T>
inline
T contrast_transfer_function<T>::compute_phase(T k_x, T k_y) const noexcept
{
	const auto xx = k_x * k_x;
	const auto yy = k_y * k_y;
	const auto squared_norm = xx + yy;
	return
		m_radial * squared_norm +
		m_axial * (xx - yy) +
		m_diagonal * (k_x * k_y) -
		m_aberration * (squared_norm * squared_norm) +
		m_phase;
}

} // namespace cpu
} // namespace rexlib
