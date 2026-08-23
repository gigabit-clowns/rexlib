// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/ndarray/const_array_ref.hpp>

#include <utility>

namespace xmipp4
{

class execution_context;

/**
 * @brief Compute the element-wise sum of two arrays.
 *
 * The inputs must be broadcast-compatible and share the same numerical
 * type. When @p out is null, the shape and type of the result are deduced
 * from the (broadcast) inputs.
 *
 * @param x The first array to be added.
 * @param y The second array to be added.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise sum.
 */
REXLIB_API
array add(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise difference of two arrays.
 *
 * The inputs must be broadcast-compatible and share the same numerical
 * type. When @p out is null, the shape and type of the result are deduced
 * from the (broadcast) inputs.
 *
 * @param x The array to be subtracted from.
 * @param y The array to subtract.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise difference x - y.
 */
REXLIB_API
array subtract(
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
 * @return array The array holding the element-wise negation -x.
 */
REXLIB_API
array negate(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise absolute value of an array.
 *
 * For complex types, the output is the real equivalent of the input type
 * (e.g. complex_float32 -> float32).
 *
 * @param x The array from which the absolute value is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise absolute value.
 */
REXLIB_API
array abs(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise product of two arrays.
 *
 * The inputs must be broadcast-compatible and share the same numerical
 * type. When @p out is null, the shape and type of the result are deduced
 * from the (broadcast) inputs.
 *
 * @param x The first array to be multiplied.
 * @param y The second array to be multiplied.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise product.
 */
REXLIB_API
array multiply(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise quotient of two arrays.
 *
 * The inputs must be broadcast-compatible and share the same numerical
 * type. When @p out is null, the shape and type of the result are deduced
 * from the (broadcast) inputs.
 *
 * @param x The dividend array.
 * @param y The divisor array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise quotient x / y.
 */
REXLIB_API
array divide(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise modulo of two arrays.
 *
 * The inputs must be broadcast-compatible and share the same numerical
 * type. When @p out is null, the shape and type of the result are deduced
 * from the (broadcast) inputs.
 *
 * @param x The dividend array.
 * @param y The divisor array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise modulo x % y.
 * 
 * @note The modulo behaves as Python's modulo and not like C's modulo. This is,
 * output has the same sign as the divisor.
 */
REXLIB_API
array modulo(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise floor division of two arrays.
 *
 * The inputs must be broadcast-compatible and share the same numerical
 * type. When @p out is null, the shape and type of the result are deduced
 * from the (broadcast) inputs.
 *
 * @param x The dividend array.
 * @param y The divisor array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise quotient, rounded down.
 *
 * @note The quotient is rounded towards negative infinity rather than
 * towards zero, so that it pairs with the remainder computed by @ref
 * modulo. Complex arrays are not accepted.
 *
 * @see modulo
 */
REXLIB_API
array floor_divide(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise sign of an array.
 *
 * @param x The array whose sign is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise sign.
 *
 * @note For complex types the result is the unit value with the same
 * argument, x / |x|, rather than the sign of the real part.
 */
REXLIB_API
array sign(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise quotient and remainder of two arrays.
 *
 * Computing both at once is cheaper than calling @ref floor_divide and
 * @ref modulo in turn, the division being the expensive half of each, and
 * the two results agree by construction.
 *
 * @param x The dividend array.
 * @param y The divisor array.
 * @param context The execution context used for dispatching.
 * @param quotient Optional output parameter to be re-used.
 * @param remainder Optional output parameter to be re-used.
 * @return std::pair<array, array> The element-wise quotient and remainder.
 *
 * @note As with @ref floor_divide the quotient is rounded towards negative
 * infinity, so that it pairs with the remainder. Complex arrays are not
 * accepted.
 *
 * @see floor_divide
 * @see modulo
 */
REXLIB_API
std::pair<array, array> divmod(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *quotient = nullptr,
	array *remainder = nullptr
);

} // namespace xmipp4
