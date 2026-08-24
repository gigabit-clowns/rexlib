// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/ops/fourier/fourier_normalization.hpp>

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>
#include <rex/core/span.hpp>

#include <cstddef>

namespace rexlib
{

class execution_context;

/**
 * @brief Transform an array to the frequency domain.
 *
 * @param x The array to be transformed.
 * @param axes The axes to transform along. Negative values refer to axes
 * from the end. The remaining axes are batch dimensions.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex spectrum.
 *
 * @note Real arrays are accepted and produce the complex spectrum of the same
 * precision. The full spectrum is stored; use @ref rfft to store only
 * what a real signal needs.
 */
REXLIB_API
array fft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array to the frequency domain.
 *
 * @param x The array to be transformed.
 * @param axes The axes to transform along. Negative values refer to axes
 * from the end. The remaining axes are batch dimensions.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex spectrum.
 *
 * @note Real arrays are accepted and produce the complex spectrum of the same
 * precision. The full spectrum is stored; use @ref rfft to store only
 * what a real signal needs.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array fft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array to the frequency domain.
 *
 * Transforms along the last axis and treats the rest as batch dimensions,
 * which is how one dimensional data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex spectrum.
 */
REXLIB_API
array fft(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array to the frequency domain.
 *
 * Transforms along the last axis and treats the rest as batch dimensions,
 * which is how one dimensional data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex spectrum.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array fft(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array to the frequency domain.
 *
 * Transforms along the last two axes and treats the rest as batch dimensions,
 * which is how image data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex spectrum.
 */
REXLIB_API
array fft2(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array to the frequency domain.
 *
 * Transforms along the last two axes and treats the rest as batch dimensions,
 * which is how image data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex spectrum.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array fft2(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array to the frequency domain.
 *
 * Transforms along the last three axes and treats the rest as batch dimensions,
 * which is how volume data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex spectrum.
 */
REXLIB_API
array fft3(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array to the frequency domain.
 *
 * Transforms along the last three axes and treats the rest as batch dimensions,
 * which is how volume data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex spectrum.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array fft3(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array back from the frequency domain.
 *
 * @param x The array to be transformed.
 * @param axes The axes to transform along. Negative values refer to axes
 * from the end. The remaining axes are batch dimensions.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex signal.
 *
 * @note The result stays complex. Use @ref irfft for a spectrum known to come
 * from a real signal.
 */
REXLIB_API
array ifft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array back from the frequency domain.
 *
 * @param x The array to be transformed.
 * @param axes The axes to transform along. Negative values refer to axes
 * from the end. The remaining axes are batch dimensions.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex signal.
 *
 * @note The result stays complex. Use @ref irfft for a spectrum known to come
 * from a real signal.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array ifft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array back from the frequency domain.
 *
 * Transforms along the last axis and treats the rest as batch dimensions,
 * which is how one dimensional data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex signal.
 */
REXLIB_API
array ifft(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array back from the frequency domain.
 *
 * Transforms along the last axis and treats the rest as batch dimensions,
 * which is how one dimensional data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex signal.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array ifft(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array back from the frequency domain.
 *
 * Transforms along the last two axes and treats the rest as batch dimensions,
 * which is how image data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex signal.
 */
REXLIB_API
array ifft2(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array back from the frequency domain.
 *
 * Transforms along the last two axes and treats the rest as batch dimensions,
 * which is how image data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex signal.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array ifft2(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array back from the frequency domain.
 *
 * Transforms along the last three axes and treats the rest as batch dimensions,
 * which is how volume data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex signal.
 */
REXLIB_API
array ifft3(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform an array back from the frequency domain.
 *
 * Transforms along the last three axes and treats the rest as batch dimensions,
 * which is how volume data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the complex signal.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array ifft3(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a real array to the frequency domain.
 *
 * @param x The array to be transformed.
 * @param axes The axes to transform along. Negative values refer to axes
 * from the end. The remaining axes are batch dimensions.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the half spectrum.
 *
 * @note Only the half that is not redundant is stored: the last transformed
 * axis holds n / 2 + 1 coefficients. Complex arrays are not accepted.
 */
REXLIB_API
array rfft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a real array to the frequency domain.
 *
 * @param x The array to be transformed.
 * @param axes The axes to transform along. Negative values refer to axes
 * from the end. The remaining axes are batch dimensions.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the half spectrum.
 *
 * @note Only the half that is not redundant is stored: the last transformed
 * axis holds n / 2 + 1 coefficients. Complex arrays are not accepted.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array rfft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a real array to the frequency domain.
 *
 * Transforms along the last axis and treats the rest as batch dimensions,
 * which is how one dimensional data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the half spectrum.
 */
REXLIB_API
array rfft(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a real array to the frequency domain.
 *
 * Transforms along the last axis and treats the rest as batch dimensions,
 * which is how one dimensional data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the half spectrum.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array rfft(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a real array to the frequency domain.
 *
 * Transforms along the last two axes and treats the rest as batch dimensions,
 * which is how image data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the half spectrum.
 */
REXLIB_API
array rfft2(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a real array to the frequency domain.
 *
 * Transforms along the last two axes and treats the rest as batch dimensions,
 * which is how image data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the half spectrum.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array rfft2(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a real array to the frequency domain.
 *
 * Transforms along the last three axes and treats the rest as batch dimensions,
 * which is how volume data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the half spectrum.
 */
REXLIB_API
array rfft3(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a real array to the frequency domain.
 *
 * Transforms along the last three axes and treats the rest as batch dimensions,
 * which is how volume data is usually laid out.
 *
 * @param x The array to be transformed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the half spectrum.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array rfft3(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a half spectrum back into a real array.
 *
 * @param x The half spectrum to be transformed.
 * @param axes The axes to transform along. Negative values refer to axes
 * from the end. The remaining axes are batch dimensions.
 * @param extent The number of samples along the last transformed axis.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the real signal.
 *
 * @note The extent of the restored axis cannot be deduced: an axis of m stored
 * coefficients comes from either 2*(m - 1) or 2*m - 1 samples. Only its
 * parity actually matters, so any extent of the right parity selects the
 * same transform.
 *
 * @see rfft
 */
REXLIB_API
array irfft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	std::size_t extent,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a half spectrum back into a real array.
 *
 * @param x The half spectrum to be transformed.
 * @param axes The axes to transform along. Negative values refer to axes
 * from the end. The remaining axes are batch dimensions.
 * @param extent The number of samples along the last transformed axis.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the real signal.
 *
 * @note The extent of the restored axis cannot be deduced: an axis of m stored
 * coefficients comes from either 2*(m - 1) or 2*m - 1 samples. Only its
 * parity actually matters, so any extent of the right parity selects the
 * same transform.
 *
 * @see rfft
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array irfft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	std::size_t extent,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a half spectrum back into a real array.
 *
 * Transforms along the last axis and treats the rest as batch dimensions.
 *
 * @param x The half spectrum to be transformed.
 * @param extent The number of samples along the last transformed axis.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the real signal.
 */
REXLIB_API
array irfft(
	const_array_ref x,
	std::size_t extent,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a half spectrum back into a real array.
 *
 * Transforms along the last axis and treats the rest as batch dimensions.
 *
 * @param x The half spectrum to be transformed.
 * @param extent The number of samples along the last transformed axis.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the real signal.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array irfft(
	const_array_ref x,
	std::size_t extent,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a half spectrum back into a real array.
 *
 * Transforms along the last two axes and treats the rest as batch dimensions.
 *
 * @param x The half spectrum to be transformed.
 * @param extent The number of samples along the last transformed axis.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the real signal.
 */
REXLIB_API
array irfft2(
	const_array_ref x,
	std::size_t extent,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a half spectrum back into a real array.
 *
 * Transforms along the last two axes and treats the rest as batch dimensions.
 *
 * @param x The half spectrum to be transformed.
 * @param extent The number of samples along the last transformed axis.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the real signal.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array irfft2(
	const_array_ref x,
	std::size_t extent,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a half spectrum back into a real array.
 *
 * Transforms along the last three axes and treats the rest as batch dimensions.
 *
 * @param x The half spectrum to be transformed.
 * @param extent The number of samples along the last transformed axis.
 * @param normalization Which of the transform pair carries the scaling.
 * A transform and its inverse undo one another whenever they are given
 * the same one.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the real signal.
 */
REXLIB_API
array irfft3(
	const_array_ref x,
	std::size_t extent,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transform a half spectrum back into a real array.
 *
 * Transforms along the last three axes and treats the rest as batch dimensions.
 *
 * @param x The half spectrum to be transformed.
 * @param extent The number of samples along the last transformed axis.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the real signal.
 *
 * @note Scales as @ref ops::fourier_normalization::backward does. Name
 * another convention through the overload that takes one.
 */
REXLIB_API
array irfft3(
	const_array_ref x,
	std::size_t extent,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Move the zero frequency of a spectrum to its centre.
 *
 * @param x The array to be shifted.
 * @param axes The axes to shift along. Negative values refer to axes
 * from the end.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the shifted array.
 */
REXLIB_API
array fftshift(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Move the zero frequency of a spectrum to its centre.
 *
 * Shifts along every axis.
 *
 * @param x The array to be shifted.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the shifted array.
 */
REXLIB_API
array fftshift(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Move the zero frequency of a spectrum back to its origin.
 *
 * @param x The array to be shifted.
 * @param axes The axes to shift along. Negative values refer to axes
 * from the end.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the shifted array.
 */
REXLIB_API
array ifftshift(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Move the zero frequency of a spectrum back to its origin.
 *
 * Shifts along every axis.
 *
 * @param x The array to be shifted.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the shifted array.
 */
REXLIB_API
array ifftshift(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rexlib
