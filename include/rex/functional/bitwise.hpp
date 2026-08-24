// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>

namespace rexlib
{

class execution_context;

/**
 * @brief Compute the element-wise bitwise conjunction of two arrays.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise bitwise conjunction.
 *
 * @note Every type with a defined bit pattern is accepted, which
 * excludes only the floating point and complex ones.
 */
REXLIB_API
array bitwise_and(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise bitwise disjunction of two arrays.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise bitwise disjunction.
 *
 * @note Every type with a defined bit pattern is accepted, which
 * excludes only the floating point and complex ones.
 */
REXLIB_API
array bitwise_or(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise bitwise exclusive disjunction of two
 * arrays.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise bitwise exclusive
 * disjunction.
 *
 * @note Every type with a defined bit pattern is accepted, which
 * excludes only the floating point and complex ones.
 */
REXLIB_API
array bitwise_xor(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Invert the bits of every element of an array.
 *
 * @param x The array whose bits are inverted.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise bitwise inversion.
 *
 * @note Every type with a defined bit pattern is accepted, which
 * excludes only the floating point and complex ones.
 */
REXLIB_API
array bitwise_not(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Shift the bits of an array leftwards.
 *
 * @param x The array whose bits are shifted.
 * @param y The number of positions to shift by.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise shifted values.
 *
 * @note Only integer arrays are accepted: the count shares the type of
 * the value being shifted, and a boolean or character count is
 * not a meaningful quantity.
 */
REXLIB_API
array left_shift(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Shift the bits of an array rightwards.
 *
 * @param x The array whose bits are shifted.
 * @param y The number of positions to shift by.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise shifted values.
 *
 * @note Only integer arrays are accepted: the count shares the type of
 * the value being shifted, and a boolean or character count is
 * not a meaningful quantity.
 */
REXLIB_API
array right_shift(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rexlib
