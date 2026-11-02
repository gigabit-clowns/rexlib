// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/ops/parameters/ctf_optics.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The contrast transfer function of one image.
 *
 * Evaluates c(k) = sin(pi L D(t) s^2 - (pi / 2) Cs L^3 s^4 + f + asin(Q))
 * at a frequency k of the image, with L the wavelength, Cs the spherical
 * aberration in angstroms, Q the amplitude contrast, f the phase shift,
 * s = |k| / (n p) the spatial frequency for an extent n and a pixel size p,
 * and D(t) the defocus along the direction t of k.
 *
 * The coefficients are derived in double precision and evaluated in T.
 *
 * @tparam T The real type the function is evaluated in.
 *
 * @see em::insert_ctf_slices_operation
 */
template <typename T>
class contrast_transfer_function
{
public:
	/**
	 * @brief Construct the function that is 0 at every frequency.
	 */
	contrast_transfer_function() noexcept;

	/**
	 * @brief Construct the function of an image.
	 *
	 * @param optics The optics of the microscope.
	 * @param pixel_size The size of a pixel, in angstroms.
	 * @param extent The number of pixels along each axis of the image.
	 * @param defocus_u The defocus along the direction of the astigmatism,
	 * in angstroms.
	 * @param defocus_v The defocus across the direction of the astigmatism,
	 * in angstroms.
	 * @param astigmatism_angle The angle of the astigmatism from the x axis
	 * towards the y axis, in radians.
	 * @param phase_shift The phase shift, in radians.
	 */
	contrast_transfer_function(
		const em::ctf_optics &optics,
		double pixel_size,
		std::size_t extent,
		double defocus_u,
		double defocus_v,
		double astigmatism_angle,
		double phase_shift
	) noexcept;

	/**
	 * @brief Evaluate the function at a frequency.
	 *
	 * @param k_x The frequency along x, in samples.
	 * @param k_y The frequency along y, in samples.
	 * @return T c(k).
	 */
	T operator()(T k_x, T k_y) const noexcept;

	/**
	 * @brief Compute the phase whose sine is the function at a frequency.
	 *
	 * @param k_x The frequency along x, in samples.
	 * @param k_y The frequency along y, in samples.
	 * @return T The phase, in radians.
	 */
	T compute_phase(T k_x, T k_y) const noexcept;

private:
	T m_radial;
	T m_axial;
	T m_diagonal;
	T m_aberration;
	T m_phase;
};

} // namespace cpu
} // namespace rexlib

#include "contrast_transfer_function.inl"
