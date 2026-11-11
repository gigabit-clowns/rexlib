// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/em/parameters/ctf_optics/contrast_transfer_function_1d.hpp>
#include <backends/cpu/em/parameters/ctf_optics/ctf_phase_coefficients.hpp>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The contrast transfer function of one image.
 *
 * Evaluates c(k) = sin(pi L D(t) s^2 - (pi / 2) Cs L^3 s^4 + f + asin(Q))
 * at a frequency k of the image, as ctf_phase_coefficients describes, with
 * D(t) the defocus along the direction t of k and f the phase shift. It is
 * the contrast_transfer_function_1d of the mean defocus, plus the phase the
 * astigmatism adds.
 *
 * The coefficients are derived in double precision and evaluated in T.
 *
 * @tparam T The real type the function is evaluated in.
 *
 * @see em::insert_ctf_slices_operation
 */
template <typename T>
class contrast_transfer_function_2d
{
public:
	/**
	 * @brief Construct the function that is 0 at every frequency.
	 */
	contrast_transfer_function_2d() noexcept;

	/**
	 * @brief Construct the function of an image.
	 *
	 * @param coefficients The coefficients of the optics and the sampling.
	 * @param defocus_u The defocus along the direction of the astigmatism,
	 * in angstroms.
	 * @param defocus_v The defocus across the direction of the astigmatism,
	 * in angstroms.
	 * @param astigmatism_angle The angle of the astigmatism from the x axis
	 * towards the y axis, in radians.
	 * @param phase_shift The phase shift, in radians.
	 */
	contrast_transfer_function_2d(
		const ctf_phase_coefficients &coefficients,
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
	contrast_transfer_function_1d<T> m_isotropic;
	T m_axial;
	T m_diagonal;
};

} // namespace cpu
} // namespace rexlib

#include "contrast_transfer_function_2d.inl"
