// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/operation.hpp>
#include <rexlib/core/dispatch/operation_descriptor.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_operation
	: public operation
{
public:
	MAKE_CONST_MOCK0(
		get_descriptor,
		const operation_descriptor&(),
		noexcept override
	);
	MAKE_CONST_MOCK0(get_name, std::string(), override);
	MAKE_CONST_MOCK0(get_arity, operation_arity(), noexcept override);
	MAKE_CONST_MOCK0(
		get_contract,
		const operation_contract&(),
		noexcept override
	);
};

} // namespace rexlib
