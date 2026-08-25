// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/operation_shape_policy.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{
namespace ops
{

/**
 * @brief Shape policy for elementwise operations.
 *
 * This policy allows broadcast compatibility of input operands.
 */
class REXLIB_API elementwise_operation_shape_policy final
	: public operation_shape_policy
{
public:
	void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

	void accept(
		const operation_descriptor &descriptor,
		span<const shape_type> user_output_shapes,
		span<const shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

	static const elementwise_operation_shape_policy& get() noexcept;
};

} // namespace ops
} // namespace rexlib

