// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/operation_shape_policy.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>

namespace rex
{
namespace ops
{

/**
 * @brief Shape policy for a batched matrix-vector product.
 *
 * The last axis of the matrix is contracted with the vector's sole axis,
 * and everything before the matrix's last two axes is broadcast against
 * everything before the vector's last axis, as an elementwise operation
 * would: `[..., m, k]` times `[..., k]` gives `[..., m]`.
 *
 * Unlike a matrix multiplication, neither operand is promoted: the matrix
 * must already have rank two or more and the vector rank one or more. That
 * rigidity is deliberate, so that the shape a caller gets is never a
 * consequence of a rank the operands happened to have; @ref vecmat and
 * @ref matmul are the operations for the other combinations.
 *
 * Stateless, and so shared by every instance.
 *
 * @see vector_matrix_shape_policy
 * @see matrix_multiply_shape_policy
 */
class REXLIB_API matrix_vector_shape_policy final
	: public operation_shape_policy
{
public:
	void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

	static const matrix_vector_shape_policy& get() noexcept;
};

} // namespace ops
} // namespace rex
