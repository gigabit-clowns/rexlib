// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/operation.hpp>
#include <rex/core/dispatch/operation_descriptor.hpp>

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
		get_operation_shape_policy,
		const operation_shape_policy&(),
		noexcept override
	);
	MAKE_CONST_MOCK0(
		get_operation_data_type_policy,
		const operation_data_type_policy&(),
		noexcept override
	);

};

} // namespace rexlib
