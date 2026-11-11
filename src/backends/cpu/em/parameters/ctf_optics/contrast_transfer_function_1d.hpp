// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/em/parameters/ctf_optics/ctf_phase_coefficients.hpp>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The contrast transfer function of a defocus, the same along every
 * direction.
 *
 * Evaluates c(k) = sin(pi L D s^2 - (pi / 2) Cs L^3 s^4 + f + asin(Q)), as
 * ctf_phase_coefficients describes, with D the defocus and f the phase
 * shift. The coefficients are derived in double precision and evaluated in
 * T.
 *
 * @tparam T The real type the function is evaluated in.
 *
 * @see em::ctf_1d_operation
 */
template <typename T>
class contrast_transfer_function_1d
{
public:
	/**
	 * @brief Construct the function that is 0 at every frequency.
	 */
	contrast_transfer_function_1d() noexcept;

	/**
	 * @brief Construct the function of a defocus.
	 *
	 * @param coefficients The coefficients of the optics and the sampling.
	 * @param defocus The defocus, in angstroms.
	 * @param phase_shift The phase shift, in radians.
	 */
	contrast_transfer_function_1d(
		const ctf_phase_coefficients &coefficients,
		double defocus,
		double phase_shift
	) noexcept;

	/**
	 * @brief Evaluate the function at a frequency.
	 *
	 * @param k The frequency, in samples.
	 * @return T c(k).
	 */
	T operator()(T k) const noexcept;

	/**
	 * @brief Compute the phase whose sine is the function at a frequency,
	 * from the square of that frequency.
	 *
	 * @param squared_frequency |k|^2, in squared samples.
	 * @return T The phase, in radians.
	 */
	T compute_squared_phase(T squared_frequency) const noexcept;

private:
	T m_defocus;
	T m_aberration;
	T m_phase;
};

} // namespace cpu
} // namespace rexlib

#include "contrast_transfer_function_1d.inl"
