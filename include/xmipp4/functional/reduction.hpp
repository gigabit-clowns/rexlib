// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/ndarray/const_array_ref.hpp>
#include <xmipp4/core/span.hpp>

#include <cstddef>

namespace xmipp4
{

class execution_context;

/**
 * @brief Sum the elements of an array along the given axes.
 *
 * @param x The array to be reduced.
 * @param axes The axes to reduce over. Negative values refer to axes from
 * the end. An empty list reduces over no axis; call the overload without
 * it to reduce over every one.
 * @param keep_dimensions Whether the reduced axes are kept with an extent
 * of one instead of being dropped.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the sums.
 *
 * @note Every type add accepts is admitted, including boolean, whose sum is a
 * disjunction. Overflow is a question about values, not about types.
 */
REXLIB_API
array sum(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Sum the elements of an array over every axis.
 *
 * @param x The array to be reduced.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the sums.
 */
REXLIB_API
array sum(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Multiply the elements of an array along the given axes.
 *
 * @param x The array to be reduced.
 * @param axes The axes to reduce over. Negative values refer to axes from
 * the end. An empty list reduces over no axis; call the overload without
 * it to reduce over every one.
 * @param keep_dimensions Whether the reduced axes are kept with an extent
 * of one instead of being dropped.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the products.
 */
REXLIB_API
array product(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Multiply the elements of an array over every axis.
 *
 * @param x The array to be reduced.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the products.
 */
REXLIB_API
array product(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Take the smallest elements of an array along the given axes.
 *
 * @param x The array to be reduced.
 * @param axes The axes to reduce over. Negative values refer to axes from
 * the end. An empty list reduces over no axis; call the overload without
 * it to reduce over every one.
 * @param keep_dimensions Whether the reduced axes are kept with an extent
 * of one instead of being dropped.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the minima.
 *
 * @note Named apart from the element-wise @ref minimum, which is a different
 * operation. Complex arrays are not accepted, there being no ordering.
 */
REXLIB_API
array amin(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Take the smallest elements of an array over every axis.
 *
 * @param x The array to be reduced.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the minima.
 */
REXLIB_API
array amin(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Take the largest elements of an array along the given axes.
 *
 * @param x The array to be reduced.
 * @param axes The axes to reduce over. Negative values refer to axes from
 * the end. An empty list reduces over no axis; call the overload without
 * it to reduce over every one.
 * @param keep_dimensions Whether the reduced axes are kept with an extent
 * of one instead of being dropped.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the maxima.
 *
 * @note Named apart from the element-wise @ref maximum, which is a different
 * operation. Complex arrays are not accepted, there being no ordering.
 */
REXLIB_API
array amax(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Take the largest elements of an array over every axis.
 *
 * @param x The array to be reduced.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the maxima.
 */
REXLIB_API
array amax(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test whether every element along the given axes is true.
 *
 * @param x The array to be reduced.
 * @param axes The axes to reduce over. Negative values refer to axes from
 * the end. An empty list reduces over no axis; call the overload without
 * it to reduce over every one.
 * @param keep_dimensions Whether the reduced axes are kept with an extent
 * of one instead of being dropped.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the tests.
 *
 * @note An element counts as true when it is non zero, so any type is accepted
 * and the result is boolean regardless.
 */
REXLIB_API
array all(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test whether every element over every axis is true.
 *
 * @param x The array to be reduced.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the tests.
 */
REXLIB_API
array all(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test whether any element along the given axes is true.
 *
 * @param x The array to be reduced.
 * @param axes The axes to reduce over. Negative values refer to axes from
 * the end. An empty list reduces over no axis; call the overload without
 * it to reduce over every one.
 * @param keep_dimensions Whether the reduced axes are kept with an extent
 * of one instead of being dropped.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the tests.
 *
 * @note An element counts as true when it is non zero, so any type is accepted
 * and the result is boolean regardless.
 */
REXLIB_API
array any(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Test whether any element over every axis is true.
 *
 * @param x The array to be reduced.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the tests.
 */
REXLIB_API
array any(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Locate the smallest elements of an array along the given axes.
 *
 * @param x The array to be reduced.
 * @param axes The axes to reduce over. Negative values refer to axes from
 * the end. An empty list reduces over no axis; call the overload without
 * it to reduce over every one.
 * @param keep_dimensions Whether the reduced axes are kept with an extent
 * of one instead of being dropped.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the indices of the minima.
 *
 * @note The result holds indices, and so is int64 whatever the input type was.
 */
REXLIB_API
array argmin(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Locate the smallest elements of an array over every axis.
 *
 * @param x The array to be reduced.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the indices of the minima.
 */
REXLIB_API
array argmin(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Locate the largest elements of an array along the given axes.
 *
 * @param x The array to be reduced.
 * @param axes The axes to reduce over. Negative values refer to axes from
 * the end. An empty list reduces over no axis; call the overload without
 * it to reduce over every one.
 * @param keep_dimensions Whether the reduced axes are kept with an extent
 * of one instead of being dropped.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the indices of the maxima.
 *
 * @note The result holds indices, and so is int64 whatever the input type was.
 */
REXLIB_API
array argmax(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Locate the largest elements of an array over every axis.
 *
 * @param x The array to be reduced.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the indices of the maxima.
 */
REXLIB_API
array argmax(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Count the non zero elements along the given axes.
 *
 * @param x The array to be reduced.
 * @param axes The axes to reduce over. Negative values refer to axes from
 * the end. An empty list reduces over no axis; call the overload without
 * it to reduce over every one.
 * @param keep_dimensions Whether the reduced axes are kept with an extent
 * of one instead of being dropped.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the counts.
 *
 * @note The result holds counts, and so is int64 whatever the input type was.
 */
REXLIB_API
array count_nonzero(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Count the non zero elements over every axis.
 *
 * @param x The array to be reduced.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the counts.
 */
REXLIB_API
array count_nonzero(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Average the elements of an array along the given axes.
 *
 * @param x The array to be reduced.
 * @param axes The axes to reduce over. Negative values refer to axes from
 * the end. An empty list reduces over no axis; call the overload without
 * it to reduce over every one.
 * @param keep_dimensions Whether the reduced axes are kept with an extent
 * of one instead of being dropped.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the averages.
 *
 * @note The result has the inexact counterpart of the input type: an integer
 * array averages to float64, while a float32 one stays float32.
 */
REXLIB_API
array mean(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Average the elements of an array over every axis.
 *
 * @param x The array to be reduced.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the averages.
 */
REXLIB_API
array mean(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

} // namespace xmipp4
