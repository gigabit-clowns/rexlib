// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/ndarray/const_array_ref.hpp>

namespace xmipp4
{

class execution_context;

/**
 * @brief Test element-wise whether an array holds values that are not numbers.
 *
 * @param x The array to be tested.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise test.
 *
 * @note Only floating point and complex arrays are accepted, no other
 * type being able to hold such a value.
 */
REXLIB_API
array is_nan(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test element-wise whether an array holds infinite values.
 *
 * @param x The array to be tested.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise test.
 *
 * @note Only floating point and complex arrays are accepted, no other
 * type being able to hold an infinity.
 */
REXLIB_API
array is_inf(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test element-wise whether an array holds finite values.
 *
 * @param x The array to be tested.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise test.
 *
 * @note Only floating point and complex arrays are accepted, no other
 * type being able to hold a non finite value.
 */
REXLIB_API
array is_finite(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test element-wise whether the sign bit of an array is set.
 *
 * @param x The array to be tested.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise test.
 *
 * @note Only floating point arrays are accepted. This distinguishes
 * negative zero from positive zero; for integers, comparing
 * against zero says the same thing more plainly.
 */
REXLIB_API
array sign_bit(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compose an array from the magnitude of one and the sign of another.
 *
 * @param x The array supplying the magnitude.
 * @param y The array supplying the sign.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise composition.
 */
REXLIB_API
array copysign(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

} // namespace xmipp4
