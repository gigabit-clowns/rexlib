// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>

namespace rex
{

class execution_context;

/**
 * @brief Raise the elements of an array to the power of another.
 *
 * The inputs must be broadcast-compatible and share the same numerical
 * type. When @p out is null, the shape and type of the result are deduced
 * from the (broadcast) inputs.
 *
 * @param x The base array.
 * @param y The exponent array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise power x ** y.
 *
 * @note A negative exponent has no representable result for integer
 * arrays, and is reported by the backend rather than rejected up front.
 */
REXLIB_API
array power(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise square of an array.
 *
 * @param x The array whose square is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise square.
 */
REXLIB_API
array square(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise reciprocal of an array.
 *
 * @param x The array whose reciprocal is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise reciprocal 1 / x.
 *
 * @note Integer arrays are not accepted: the reciprocal of every integer of
 * magnitude above one is zero.
 */
REXLIB_API
array reciprocal(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise square root of an array.
 *
 * @param x The array whose square root is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise square root.
 *
 * @note The square root of a negative real is not representable in the operand
 * type and yields NaN. Cast the input to a complex type first to obtain
 * the complex branch.
 */
REXLIB_API
array sqrt(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise cube root of an array.
 *
 * @param x The array whose cube root is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise cube root.
 *
 * @note Unlike @ref sqrt this is defined for negative arrays, and accepts only
 * real floating point ones.
 */
REXLIB_API
array cbrt(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise exponential of an array.
 *
 * @param x The array whose exponential is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise exponential.
 */
REXLIB_API
array exp(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise base two exponential of an array.
 *
 * @param x The array whose exponential is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise base two exponential.
 */
REXLIB_API
array exp2(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise exponential of an array, minus one.
 *
 * @param x The array whose exponential is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise exponential minus one.
 *
 * @note Accurate for arrays close to zero, where computing the exponential and
 * subtracting one loses the significant digits.
 */
REXLIB_API
array expm1(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise natural logarithm of an array.
 *
 * @param x The array whose logarithm is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise natural logarithm.
 */
REXLIB_API
array log(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise base two logarithm of an array.
 *
 * @param x The array whose logarithm is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise base two logarithm.
 */
REXLIB_API
array log2(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise base ten logarithm of an array.
 *
 * @param x The array whose logarithm is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise base ten logarithm.
 */
REXLIB_API
array log10(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise natural logarithm of one plus an array.
 *
 * @param x The array to which one is added.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise logarithm of one plus x.
 *
 * @note Accurate for arrays close to zero, where adding one first loses the
 * significant digits.
 */
REXLIB_API
array log1p(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rex
