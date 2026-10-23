// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/operation_contract.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_operation_contract
	: public operation_contract
{
public:
	MAKE_CONST_MOCK3(
		deduce,
		void(
			const operation_descriptor&,
			span<operand_specification> canonical_outputs,
			span<const operand_specification> inputs
		),
		override
	);
	MAKE_CONST_MOCK4(
		accept,
		void(
			const operation_descriptor&,
			span<const operand_specification> user_outputs,
			span<const operand_specification> canonical_outputs,
			span<const operand_specification> inputs
		),
		override
	);
};

} // namespace rexlib
