// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/operation_shape_policy.hpp>
#include <rex/core/platform/dynamic_shared_object.h>

namespace rex
{
namespace ops
{

/**
 * @brief Shape policy for a batched vector-matrix product.
 *
 * The vector's sole axis is contracted with the matrix's second to last
 * axis, and everything before the vector's axis is broadcast against
 * everything before the matrix's last two axes: `[..., k]` times
 * `[..., k, n]` gives `[..., n]`.
 *
 * The mirror image of @ref matrix_vector_shape_policy: neither operand is
 * promoted, so the vector must already have rank one or more and the
 * matrix rank two or more.
 *
 * Stateless, and so shared by every instance.
 *
 * @see matrix_vector_shape_policy
 * @see matrix_multiply_shape_policy
 */
class REXLIB_API vector_matrix_shape_policy final
	: public operation_shape_policy
{
public:
	void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

	static const vector_matrix_shape_policy& get() noexcept;
};

} // namespace ops
} // namespace rex
