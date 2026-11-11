// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/ops/parameters/ctf_optics.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The coefficients of the phase of the contrast transfer function,
 * for some optics and a sampling of the frequencies.
 *
 * At a frequency of k samples, the phase is
 * defocus_scale D |k|^2 - aberration |k|^4 + amplitude_phase + f, with D
 * the defocus along k and f the phase shift: pi L D s^2 - (pi / 2) Cs L^3
 * s^4 + asin(Q) + f, with L the wavelength, Cs the spherical aberration in
 * angstroms, Q the amplitude contrast and s = |k| / (n p) the spatial
 * frequency for an extent n and a pixel size p.
 */
class ctf_phase_coefficients
{
public:
	/**
	 * @brief Derive the coefficients of some optics and sampling.
	 *
	 * @param optics The optics of the microscope.
	 * @param pixel_size The size of a pixel, in angstroms.
	 * @param extent The number of pixels n along each axis.
	 */
	ctf_phase_coefficients(
		const em::ctf_optics &optics,
		double pixel_size,
		std::size_t extent
	) noexcept;

	/**
	 * @brief Get the factor of the defocus and of |k|^2.
	 *
	 * @return double pi L / (n p)^2, per angstrom of defocus.
	 */
	double get_defocus_scale() const noexcept;

	/**
	 * @brief Get the factor of |k|^4.
	 *
	 * @return double (pi / 2) Cs L^3 / (n p)^4.
	 */
	double get_aberration() const noexcept;

	/**
	 * @brief Get the phase the amplitude contrast adds.
	 *
	 * @return double asin(Q), in radians.
	 */
	double get_amplitude_phase() const noexcept;

private:
	double m_defocus_scale;
	double m_aberration;
	double m_amplitude_phase;
};

} // namespace cpu
} // namespace rexlib
