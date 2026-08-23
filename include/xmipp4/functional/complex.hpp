// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/ndarray/const_array_ref.hpp>

namespace rex
{

class execution_context;

/**
 * @brief Obtain a view of the real part of an array.
 *
 * Nothing is computed and nothing is allocated. A complex array is an array
 * of twice as many real values, std::complex being layout compatible with
 * an array of two of its value type, so the real parts are reachable by
 * doubling the strides. The result therefore aliases @p x, and writing
 * through it writes into @p x.
 *
 * A real array is already its own real part and is returned as a plain
 * alias.
 *
 * Being a view rather than an operation, this needs no execution context
 * and reaches no backend. Call @ref array::share_const on the result for a
 * read-only alias.
 *
 * @param x The array whose real part is viewed.
 * @return array A view of the real part of @p x.
 *
 * @see imag
 */
REXLIB_API
array real(array &x);

/**
 * @brief Obtain a view of the imaginary part of an array.
 *
 * As with @ref real, nothing is computed and the result aliases @p x.
 *
 * Unlike @ref real this is not defined for every array: the imaginary part
 * of a real array is zero everywhere, and no view can produce a value the
 * storage does not hold. Build it with @ref zeros instead.
 *
 * @param x The array whose imaginary part is viewed. Must be complex.
 * @return array A view of the imaginary part of @p x.
 *
 * @throws std::invalid_argument If @p x is not complex.
 *
 * @see real
 */
REXLIB_API
array imag(array &x);

/**
 * @brief Compute the element-wise conjugate of an array.
 *
 * @param x The array to be conjugated.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise conjugate.
 *
 * @note Real arrays are accepted and left unchanged, so that code generic
 * over the element type needs no special case.
 */
REXLIB_API
array conjugate(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Compute the element-wise argument of an array.
 *
 * @param x The array whose argument is computed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the element-wise argument, in radians.
 *
 * @note The result has the real counterpart of the input type, an angle
 * having no imaginary part. Integer arrays are not accepted: the argument
 * of an integer is a degenerate choice between zero and pi.
 */
REXLIB_API
array angle(
	const_array_ref x,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rex
