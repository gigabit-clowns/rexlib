// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{

class execution_context;

namespace em
{

/**
 * @brief Standardize images against the statistics of their background.
 *
 * @param value The images. Their axes are the named ones, and every other
 * axis is a batch axis.
 * @param seed The uint64 seeds of the draws replacing elements. It has no
 * image axes and broadcasts against the batch axes of @p value.
 * @param axes The axes of an image. Negative values refer to axes from the
 * end.
 * @param radius The radius of the region excluded from the background, in
 * elements.
 * @param lower_bound The lowest standardized value kept, in standard
 * deviations.
 * @param upper_bound The highest standardized value kept, in standard
 * deviations.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The standardized images.
 *
 * @see standardize_background_operation
 */
REXLIB_API
array standardize_background(
	const_array_ref value,
	const_array_ref seed,
	span<const std::ptrdiff_t> axes,
	double radius,
	double lower_bound,
	double upper_bound,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Standardize images held in the last two axes against the
 * statistics of their background.
 *
 * @param value The images, in its last two axes. Every other axis is a
 * batch axis.
 * @param seed The uint64 seeds of the draws replacing elements. It has no
 * image axes and broadcasts against the batch axes of @p value.
 * @param radius The radius of the region excluded from the background, in
 * elements.
 * @param lower_bound The lowest standardized value kept, in standard
 * deviations.
 * @param upper_bound The highest standardized value kept, in standard
 * deviations.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The standardized images.
 *
 * @throws std::invalid_argument When @p value has fewer than two axes.
 *
 * @see standardize_background_operation
 */
REXLIB_API
array standardize_background_2d(
	const_array_ref value,
	const_array_ref seed,
	double radius,
	double lower_bound,
	double upper_bound,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Standardize volumes held in the last three axes against the
 * statistics of their background.
 *
 * @param value The volumes, in its last three axes. Every other axis is a
 * batch axis.
 * @param seed The uint64 seeds of the draws replacing elements. It has no
 * volume axes and broadcasts against the batch axes of @p value.
 * @param radius The radius of the region excluded from the background, in
 * elements.
 * @param lower_bound The lowest standardized value kept, in standard
 * deviations.
 * @param upper_bound The highest standardized value kept, in standard
 * deviations.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The standardized volumes.
 *
 * @throws std::invalid_argument When @p value has fewer than three axes.
 *
 * @see standardize_background_operation
 */
REXLIB_API
array standardize_background_3d(
	const_array_ref value,
	const_array_ref seed,
	double radius,
	double lower_bound,
	double upper_bound,
	const execution_context &context,
	array *out = nullptr
);

} // namespace em
} // namespace rexlib
