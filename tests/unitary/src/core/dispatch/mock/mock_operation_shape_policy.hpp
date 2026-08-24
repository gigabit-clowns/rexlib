// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/operation_shape_policy.hpp>

#include <rexlib/core/dispatch/operation_descriptor.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_operation_shape_policy
	: public operation_shape_policy
{
public:
	using shape_type = operation_shape_policy::shape_type;

	MAKE_CONST_MOCK3(
		deduce,
		void(
			const operation_descriptor&,
			span<shape_type> canonical_output_shapes,
			span<const shape_type> input_shapes
		),
		override
	);
	MAKE_CONST_MOCK4(
		accept,
		void(
			const operation_descriptor&,
			span<const shape_type> user_output_shapes,
			span<const shape_type> canonical_output_shapes,
			span<const shape_type> input_shapes
		),
		override
	);
};

} // namespace rexlib
