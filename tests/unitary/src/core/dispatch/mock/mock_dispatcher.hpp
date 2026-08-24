// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/dispatcher.hpp>

#include <rexlib/core/dispatch/operation.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/hardware/device_context.hpp>
#include <rexlib/core/hardware/command_queue.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_dispatcher final
	: public dispatcher
{
public:
	MAKE_MOCK4(
		dispatch,
		void (
			const operation &operation,
			span<array> output_operands,
			span<const const_array_ref> input_operands,
			const device_context &device_context
		),
		override
	);
};

} // namespace rexlib
