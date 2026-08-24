// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>

namespace rexlib
{

class execution_context;

/**
 * @brief Round the elements of an array downwards.
 *
 * @param x The array to be rounded.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise floor.
 *
 * @note Integer arrays are not accepted, rounding one being a copy.
 */
REXLIB_API
array floor(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Round the elements of an array upwards.
 *
 * @param x The array to be rounded.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise ceiling.
 */
REXLIB_API
array ceil(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Round the elements of an array towards zero.
 *
 * @param x The array to be rounded.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise truncation.
 */
REXLIB_API
array trunc(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Round the elements of an array to the nearest integer.
 *
 * @param x The array to be rounded.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise rounding.
 *
 * @note Ties are resolved towards the even neighbour, as IEEE 754 requires,
 * rather than away from zero.
 */
REXLIB_API
array round(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rexlib
