// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>

#include <utility>

namespace rex
{

class execution_context;

/**
 * @brief Compute the element-wise sine of an array.
 *
 * @param x The array whose sine is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise sine.
 */
REXLIB_API
array sin(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise cosine of an array.
 *
 * @param x The array whose cosine is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise cosine.
 */
REXLIB_API
array cos(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise tangent of an array.
 *
 * @param x The array whose tangent is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise tangent.
 */
REXLIB_API
array tan(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise arc sine of an array.
 *
 * @param x The array whose arc sine is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise arc sine.
 */
REXLIB_API
array asin(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise arc cosine of an array.
 *
 * @param x The array whose arc cosine is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise arc cosine.
 */
REXLIB_API
array acos(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise arc tangent of an array.
 *
 * @param x The array whose arc tangent is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise arc tangent.
 */
REXLIB_API
array atan(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise hyperbolic sine of an array.
 *
 * @param x The array whose hyperbolic sine is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise hyperbolic sine.
 */
REXLIB_API
array sinh(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise hyperbolic cosine of an array.
 *
 * @param x The array whose hyperbolic cosine is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise hyperbolic cosine.
 */
REXLIB_API
array cosh(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise hyperbolic tangent of an array.
 *
 * @param x The array whose hyperbolic tangent is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise hyperbolic tangent.
 */
REXLIB_API
array tanh(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise inverse hyperbolic sine of an array.
 *
 * @param x The array whose inverse hyperbolic sine is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise inverse hyperbolic sine.
 */
REXLIB_API
array asinh(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise inverse hyperbolic cosine of an array.
 *
 * @param x The array whose inverse hyperbolic cosine is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise inverse hyperbolic cosine.
 */
REXLIB_API
array acosh(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise inverse hyperbolic tangent of an array.
 *
 * @param x The array whose inverse hyperbolic tangent is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise inverse hyperbolic tangent.
 */
REXLIB_API
array atanh(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise arc tangent of two arrays, by quadrant.
 *
 * @param y The ordinate array.
 * @param x The abscissa array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise arc tangent of y / x.
 *
 * @note The ordinate comes first and the abscissa second, matching
 * std::atan2 rather than reading left to right.
 */
REXLIB_API
array atan2(
	const_array_ref y,
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise hypotenuse of two arrays.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise hypotenuse.
 *
 * @note Computed without the intermediate overflow that squaring the
 * inputs would cause.
 */
REXLIB_API
array hypot(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Convert an array from radians to degrees.
 *
 * @param x The array to be converted, in radians.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the converted angles, in degrees.
 */
REXLIB_API
array degrees(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Convert an array from degrees to radians.
 *
 * @param x The array to be converted, in degrees.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the converted angles, in radians.
 */
REXLIB_API
array radians(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise sine and cosine of an array at once.
 *
 * Cheaper than calling @ref sin and @ref cos in turn, the two sharing the
 * argument reduction that dominates their cost. Rotations and projections
 * need both.
 *
 * @param x The array whose sine and cosine are computed.
 * @param context The execution context used for dispatching.
 * @param sine Optional output parameter to be re-used.
 * @param cosine Optional output parameter to be re-used.
 * @return std::pair<array, array> The element-wise sine and cosine.
 *
 * @see sin
 * @see cos
 */
REXLIB_API
std::pair<array, array> sincos(
	const_array_ref x,
	const execution_context &context,
	array *sine = nullptr,
	array *cosine = nullptr
);

} // namespace rex
