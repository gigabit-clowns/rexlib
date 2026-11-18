// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/device.hpp>

#include <memory>

namespace sample
{

/**
 * @brief A device over host memory whose queues run each program on the
 * thread that submits it.
 */
class sample_device final
	: public rexlib::device
{
public:
	const rexlib::memory_resource&
	get_memory_resource(
		rexlib::memory_resource_affinity affinity
	) const override;

	std::shared_ptr<rexlib::command_queue>
	create_command_queue() const override;
};

} // namespace sample
