// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <ostream>

namespace rex
{
namespace ops
{

/**
 * @brief Which of a transform pair carries the scaling.
 *
 * A discrete Fourier transform followed by its inverse multiplies a signal by
 * the number of samples, so one of the two has to divide it back out. Which
 * one is a convention rather than a fact, and this is it.
 *
 * The convention is named rather than the factor itself so that a transform
 * and its inverse undo one another whenever they are given the same one. A
 * caller that names a factor per transform instead is left to pair the two up
 * correctly, and pairing them wrongly is silent.
 *
 * There is deliberately no "none": it would be @ref backward on a forward
 * transform and @ref forward on an inverse one, so it names no scaling that
 * cannot already be asked for, and two spellings of one operation are two
 * programs where there should be one.
 */
enum class fourier_normalization
{
	/**
	 * @brief The inverse transform divides by the number of samples.
	 *
	 * The forward transform is left as it is computed. This is what NumPy,
	 * SciPy and the array API all do when no convention is named.
	 */
	backward,

	/**
	 * @brief Both transforms divide by the square root of the count.
	 *
	 * The only convention under which the transform preserves the total
	 * energy of the signal, so that Parseval's identity holds without a
	 * factor, and the only one that is its own mirror image.
	 */
	ortho,

	/**
	 * @brief The forward transform divides by the number of samples.
	 *
	 * The inverse is then left as it is computed. Makes the zero frequency
	 * of a forward transform the mean of the signal rather than its total.
	 */
	forward
};

/**
 * @brief Which way round a transform goes.
 *
 * Half of what a scale factor depends on, the other half being the
 * convention. It is a property of the operation rather than a parameter of
 * it: there is no transform that is forward or inverse depending on what it
 * is told.
 */
enum class fourier_direction
{
	/// From the signal domain to the frequency domain.
	forward,

	/// From the frequency domain back to the signal domain.
	inverse
};

REXLIB_API
const char* to_string(fourier_normalization normalization) noexcept;

REXLIB_API
std::ostream& operator<<(
	std::ostream &os,
	fourier_normalization normalization
);

REXLIB_API
const char* to_string(fourier_direction direction) noexcept;

REXLIB_API
std::ostream& operator<<(std::ostream &os, fourier_direction direction);

/**
 * @brief Get the factor a transform multiplies its result by.
 *
 * Exported because every backend needs it and none of them is given the
 * convention itself: pocketfft and MKL take a factor, FFTW and cuFFT take
 * nothing at all and have to apply one themselves. Deriving it here, once,
 * is what keeps a transform meaning the same thing whichever of them runs
 * it.
 *
 * @param normalization Which of the transform pair carries the scaling.
 * @param direction Which way round this transform goes.
 * @param sample_count The number of samples reaching each transformed value,
 * which is the product of the **signal** extents along the transformed axes.
 * The signal is the operand that is not a half spectrum, so a real transform
 * and its inverse are both measured against the real operand rather than
 * against the coefficients actually stored. A count of zero returns one,
 * there being nothing to scale.
 * @return double The factor.
 */
REXLIB_API
double get_fourier_scale(
	fourier_normalization normalization,
	fourier_direction direction,
	std::size_t sample_count
) noexcept;

} // namespace ops
} // namespace rex
