// SPDX-License-Identifier: GPL-3.0-only

#include "sample_device.hpp"

#include "sample_command_queue.hpp"

#include <rexlib/core/hardware/memory_resource.hpp>

namespace sample
{

const rexlib::memory_resource& sample_device::get_memory_resource(
	rexlib::memory_resource_affinity /*affinity*/
) const
{
	return rexlib::get_host_memory_resource();
}

std::shared_ptr<rexlib::command_queue>
sample_device::create_command_queue() const
{
	return std::make_shared<sample_command_queue>();
}

} // namespace sample
