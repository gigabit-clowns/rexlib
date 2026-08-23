// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/operation.hpp>
#include <xmipp4/core/dispatch/operation_descriptor.hpp>

#include <trompeloeil.hpp>

namespace rex
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

} // namespace rex
