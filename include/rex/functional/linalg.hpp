// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>

#include <cstddef>

namespace rex
{

class execution_context;

/**
 * @brief Multiply two arrays as stacks of matrices.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the product.
 *
 * @note The last two axes of each input are the matrix and the rest are a stack
 * of them, broadcast between the inputs. A one dimensional input is
 * promoted for the duration and the axis added is dropped again.
 */
REXLIB_API
array matmul(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Multiply a stack of matrices by a stack of vectors.
 *
 * @param x The stack of matrices.
 * @param y The stack of vectors.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the products.
 *
 * @note The matrix's last axis is contracted with the vector's last axis,
 * and everything before is a stack broadcast between the operands, as
 * with @ref matmul. Unlike @ref matmul, neither operand is promoted: @p x
 * must already have rank two or more and @p y rank one or more.
 *
 * @see vecmat
 * @see matmul
 */
REXLIB_API
array matvec(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Multiply a stack of vectors by a stack of matrices.
 *
 * @param x The stack of vectors.
 * @param y The stack of matrices.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the products.
 *
 * @note The vector's last axis is contracted with the matrix's second to
 * last axis, and everything before is a stack broadcast between the
 * operands. The mirror image of @ref matvec: neither operand is promoted,
 * so @p x must already have rank one or more and @p y rank two or more.
 *
 * @see matvec
 * @see matmul
 */
REXLIB_API
array vecmat(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Contract two arrays along one axis, broadcasting the rest.
 *
 * @param x The first array.
 * @param y The second array.
 * @param axis The axis to contract along. Negative values refer to axes
 * from the end.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the contracted products.
 *
 * @note The inputs are broadcast together and the contracted axis is lost.
 * This differs from @ref matmul, which takes the last two axes as a
 * matrix.
 *
 * @see matmul
 */
REXLIB_API
array vecdot(
	const_array_ref x,
	const_array_ref y,
	std::ptrdiff_t axis,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Contract two arrays along their last axis.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the contracted products.
 */
REXLIB_API
array vecdot(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Take the cross product of two arrays along one axis.
 *
 * @param x The first array.
 * @param y The second array.
 * @param axis The axis to take the product along. Negative values refer to
 * axes from the end. It must have three components.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the cross products.
 *
 * @note The inputs are broadcast together, so a stack of vectors crosses
 * with a single one without either being written out.
 */
REXLIB_API
array cross(
	const_array_ref x,
	const_array_ref y,
	std::ptrdiff_t axis,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Take the cross product of two arrays along their last axis.
 *
 * @param x The first array.
 * @param y The second array.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The array holding the cross products.
 */
REXLIB_API
array cross(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rex
