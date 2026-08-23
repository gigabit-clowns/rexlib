// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/backends/cpu/program_builder.hpp>

#include <xmipp4/core/dispatch/operand_signature.hpp>
#include <xmipp4/core/hardware/memory_resource.hpp>
#include <backends/cpu/hardware/command_queue.hpp>

#include <algorithm>

namespace rex
{
namespace cpu
{

namespace
{

bool check_host_access(const operand_signature &signature) noexcept
{
	const auto *resource = signature.get_memory_resource();
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

bool check_array_signatures(
	span<const operand_signature> signatures
) noexcept
{
	return std::all_of(signatures.begin(), signatures.end(), check_host_access);
}

} // anonymous namespace


backend_priority program_builder::get_suitability(
	const operation& /*operation*/,
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures,
	xmipp4::command_queue &queue
) const
{
	if (!check_array_signatures(output_signatures))
	{
		return backend_priority::unsupported;
	}

	if (!check_array_signatures(input_signatures))
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
} // namespace rex

