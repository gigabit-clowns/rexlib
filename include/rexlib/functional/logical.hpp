// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>

namespace rexlib
{

class execution_context;

/**
 * @brief Compute the element-wise conjunction of two arrays.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise conjunction.
 *
 * @note Arrays of any type are accepted, an element counting as true
 * when it is non zero, and the result is boolean regardless.
 */
REXLIB_API
array logical_and(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise disjunction of two arrays.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise disjunction.
 *
 * @note Arrays of any type are accepted, an element counting as true
 * when it is non zero, and the result is boolean regardless.
 */
REXLIB_API
array logical_or(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise exclusive disjunction of two arrays.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise exclusive disjunction.
 *
 * @note Arrays of any type are accepted, an element counting as true
 * when it is non zero, and the result is boolean regardless.
 */
REXLIB_API
array logical_xor(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise negation of an array.
 *
 * @param x The array to be negated.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise negation.
 *
 * @note Arrays of any type are accepted, an element counting as true
 * when it is non zero, and the result is boolean regardless.
 */
REXLIB_API
array logical_not(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rexlib
