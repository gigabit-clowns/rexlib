// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/backends/cpu/program_builder.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <backends/cpu/hardware/command_queue.hpp>

#include <algorithm>

namespace rexlib
{
namespace cpu
{

namespace
{

bool check_host_access(const operand_descriptor &descriptor) noexcept
{
	const auto *resource = descriptor.get_memory_resource();
	if (!resource)
	{
		return false;
	}

	if (!is_host_accessible(resource->get_kind()))
	{
		return false;
	}

	return true;
}

bool check_array_descriptors(
	span<const operand_descriptor> descriptors
) noexcept
{
	return std::all_of(
		descriptors.begin(),
		descriptors.end(),
		check_host_access
	);
}

} // anonymous namespace


backend_priority program_builder::get_suitability(
	const operation& /*operation*/,
	span<const operand_descriptor> output_descriptors,
	span<const operand_descriptor> input_descriptors,
	rexlib::command_queue &queue
) const
{
	if (!check_array_descriptors(output_descriptors))
	{
		return backend_priority::unsupported;
	}

	if (!check_array_descriptors(input_descriptors))
	{
		return backend_priority::unsupported;
	}

	if (!command_queue::try_cast(queue))
	{
		return backend_priority::unsupported;
	}

	return backend_priority::normal;
}

} // namespace cpu
} // namespace rexlib

