// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/em/parameters/ctf_optics/ctf_phase_coefficients.hpp>

#include <cmath>

#include <boost/math/constants/constants.hpp>

namespace rexlib
{
namespace cpu
{

ctf_phase_coefficients::ctf_phase_coefficients(
	const em::ctf_optics &optics,
	double pixel_size,
	std::size_t extent
) noexcept
	: m_amplitude_phase(std::asin(optics.get_amplitude_contrast()))
{
	using boost::math::double_constants::pi;

	const auto angstroms_per_millimetre = 1e7;
	const auto wavelength = optics.get_wavelength();
	const auto aberration =
		optics.get_spherical_aberration() * angstroms_per_millimetre;
	const auto frequency_scale =
		1.0 / (static_cast<double>(extent) * pixel_size);
	const auto squared_scale = frequency_scale * frequency_scale;

	m_defocus_scale = pi * wavelength * squared_scale;
	m_aberration = 0.5 * pi * aberration * wavelength * wavelength *
		wavelength * squared_scale * squared_scale;
}

double ctf_phase_coefficients::get_defocus_scale() const noexcept
{
	return m_defocus_scale;
}

double ctf_phase_coefficients::get_aberration() const noexcept
{
	return m_aberration;
}

double ctf_phase_coefficients::get_amplitude_phase() const noexcept
{
	return m_amplitude_phase;
}

} // namespace cpu
} // namespace rexlib
