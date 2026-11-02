// SPDX-License-Identifier: GPL-3.0-only

#include "contrast_transfer_function.hpp"

#include "pi.hpp"

#include <cmath>

namespace rexlib
{
namespace cpu
{

namespace
{

REXLIB_CONST_CONSTEXPR double angstroms_per_millimetre = 1e7;

} // anonymous namespace

contrast_transfer_function::contrast_transfer_function(
	const em::ctf_optics &optics,
	double pixel_size,
	std::size_t extent,
	double defocus_u,
	double defocus_v,
	double astigmatism_angle,
	double phase_shift
) noexcept
{
	const auto wavelength = optics.get_wavelength();
	const auto aberration =
		optics.get_spherical_aberration() * angstroms_per_millimetre;
	const auto frequency_scale =
		1.0 / (static_cast<double>(extent) * pixel_size);
	const auto squared_scale = frequency_scale * frequency_scale;

	m_defocus_scale = pi * wavelength * squared_scale;
	m_aberration_scale = 0.5 * pi * aberration *
		wavelength * wavelength * wavelength * squared_scale * squared_scale;
	m_mean_defocus = 0.5 * (defocus_u + defocus_v);
	m_half_defocus_difference = 0.5 * (defocus_u - defocus_v);
	m_cos_double_angle = std::cos(2.0 * astigmatism_angle);
	m_sin_double_angle = std::sin(2.0 * astigmatism_angle);
	m_phase = phase_shift + std::asin(optics.get_amplitude_contrast());
}

double contrast_transfer_function::operator()(
	double k_x,
	double k_y
) const noexcept
{
	const auto xx = k_x * k_x;
	const auto yy = k_y * k_y;
	const auto squared_norm = xx + yy;

	// D(t) |k|^2 with cos 2t = (k_x^2 - k_y^2) / |k|^2 and
	// sin 2t = 2 k_x k_y / |k|^2.
	const auto defocus_term =
		m_mean_defocus * squared_norm +
		m_half_defocus_difference * (
			(xx - yy) * m_cos_double_angle +
			2.0 * k_x * k_y * m_sin_double_angle
		);

	const auto argument =
		m_defocus_scale * defocus_term -
		m_aberration_scale * squared_norm * squared_norm +
		m_phase;
	return std::sin(argument);
}

} // namespace cpu
} // namespace rexlib
