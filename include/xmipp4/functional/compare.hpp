// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>

namespace rex
{

class execution_context;

/**
 * @brief Test element-wise whether one array is equal to another.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise comparison.
 */
REXLIB_API
array equal(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test element-wise whether one array is different from another.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise comparison.
 */
REXLIB_API
array not_equal(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test element-wise whether one array is less than another.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise comparison.
 *
 * @note Complex arrays are not accepted, the complex plane having no
 * ordering.
 */
REXLIB_API
array less(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test element-wise whether one array is less than or equal to
 * another.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise comparison.
 *
 * @note Complex arrays are not accepted, the complex plane having no
 * ordering.
 */
REXLIB_API
array less_equal(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test element-wise whether one array is greater than another.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise comparison.
 *
 * @note Complex arrays are not accepted, the complex plane having no
 * ordering.
 */
REXLIB_API
array greater(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test element-wise whether one array is greater than or equal
 * to another.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise comparison.
 *
 * @note Complex arrays are not accepted, the complex plane having no
 * ordering.
 */
REXLIB_API
array greater_equal(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rex
