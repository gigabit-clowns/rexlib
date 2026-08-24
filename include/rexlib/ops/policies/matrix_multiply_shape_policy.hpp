// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/operation_shape_policy.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{
namespace ops
{

/**
 * @brief Shape policy for a batched matrix multiplication.
 *
 * The last two axes of each operand are the matrix, and everything before
 * them is a stack of matrices which is broadcast between the operands. So
 * `[..., m, k]` times `[..., k, n]` gives `[..., m, n]`, with the leading
 * axes broadcast together as an elementwise operation would.
 *
 * A one dimensional operand is promoted for the duration: the first is
 * treated as a row and the second as a column, and the axis added to do so
 * is removed from the result. That is what makes a matrix times a vector,
 * and a vector times a vector, spell the same way as the general case.
 *
 * Stateless, and so shared by every instance.
 */
class REXLIB_API matrix_multiply_shape_policy final
	: public operation_shape_policy
{
public:
	void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

	static const matrix_multiply_shape_policy& get() noexcept;
};

} // namespace ops
} // namespace rexlib
