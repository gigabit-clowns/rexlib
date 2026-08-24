// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>

namespace rexlib
{

class execution_context;

/**
 * @brief Select element-wise between two arrays according to a condition.
 *
 * @param condition The boolean array selecting between the other two.
 * @param x The array selected where the condition holds.
 * @param y The array selected where it does not.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise selection.
 *
 * @note The condition must be a boolean array. All three inputs must be
 * broadcast-compatible.
 */
REXLIB_API
array where(
	const_array_ref condition,
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Confine the elements of an array to an interval.
 *
 * @param x The array to be confined.
 * @param lower The lower bound.
 * @param upper The upper bound.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise confined values.
 *
 * @note The bounds are arrays rather than scalars, so they broadcast
 * against the value and may vary across it. Complex arrays are
 * not accepted, there being no ordering to clamp against.
 */
REXLIB_API
array clip(
	const_array_ref x,
	const_array_ref lower,
	const_array_ref upper,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Take the element-wise maximum of two arrays.
 *
 * Selects, element by element, whichever of the two operands is greater,
 * which is the lower half of a @ref clip taken on its own.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise maximum.
 *
 * @note Complex arrays are not accepted, the complex plane having no
 * ordering.
 */
REXLIB_API
array maximum(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Take the element-wise minimum of two arrays.
 *
 * Selects, element by element, whichever of the two operands is smaller,
 * which is the upper half of a @ref clip taken on its own.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise minimum.
 *
 * @note Complex arrays are not accepted, the complex plane having no
 * ordering.
 */
REXLIB_API
array minimum(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rexlib
