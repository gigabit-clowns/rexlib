// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/operation_data_type_policy.hpp>

#include <rex/core/dispatch/operation_descriptor.hpp>

#include <trompeloeil.hpp>

namespace rex
{

class mock_operation_data_type_policy
	: public operation_data_type_policy
{
public:
	MAKE_CONST_MOCK3(
		deduce,
		void(
			const operation_descriptor&,
			span<numerical_type> canonical_output_types,
			span<const numerical_type> input_types
		),
		override
	);
	MAKE_CONST_MOCK4(
		accept,
		void(
			const operation_descriptor&,
			span<const numerical_type> user_output_types,
			span<const numerical_type> canonical_output_types,
			span<const numerical_type> input_types
		),
		override
	);
};

} // namespace rex
