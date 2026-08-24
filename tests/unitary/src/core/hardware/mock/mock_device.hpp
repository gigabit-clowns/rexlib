// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/device.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_device final
	: public device
{
public:
	mock_device() = default;

	MAKE_CONST_MOCK1(
		get_memory_resource,
		const memory_resource&(memory_resource_affinity affinity),
		override
	);
	MAKE_CONST_MOCK0(
		create_command_queue,
		std::shared_ptr<command_queue>(),
		override
	);
	MAKE_CONST_MOCK1(
		create_event,
		std::shared_ptr<event>(event_usage_flags usage),
		override
	);
};

} // namespace rexlib
