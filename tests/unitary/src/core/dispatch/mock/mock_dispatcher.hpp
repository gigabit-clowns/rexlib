// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/dispatcher.hpp>

#include <xmipp4/core/dispatch/operation.hpp>
#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/ndarray/const_array_ref.hpp>
#include <xmipp4/core/hardware/device_context.hpp>
#include <xmipp4/core/hardware/command_queue.hpp>

#include <trompeloeil.hpp>

namespace rex
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

} // namespace rex
