// SPDX-License-Identifier: GPL-3.0-only

#include "sample_arange_program_builder.hpp"

#include "sample_arange_program.hpp"
#include "sample_command_queue.hpp"
#include "sample_program_builder_registry.hpp"

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/dispatch/program_builder_registration.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/hardware/memory_resource_kind.hpp>
#include <rexlib/core/numerical/scalar_value.hpp>
#include <rexlib/core/operation/operation_cast.hpp>
#include <rexlib/ops/creation/arange_operation.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace sample
{

namespace
{

const rexlib::program_builder_registration<sample_arange_program_builder>
	registration(get_sample_program_builder_registry());

bool has_host_accessible_memory(
	const rexlib::operand_descriptor &descriptor
) noexcept
{
	const auto *resource = descriptor.get_memory_resource();
	return resource != nullptr &&
		rexlib::is_host_accessible(resource->get_kind());
}

} // anonymous namespace

rexlib::operation_id
sample_arange_program_builder::get_operation_id() const noexcept
{
	return rexlib::operation_id::of<rexlib::ops::arange_operation>();
}

rexlib::backend_priority sample_arange_program_builder::get_suitability(
	const rexlib::operation & /*operation*/,
	rexlib::span<const rexlib::operand_descriptor> output_descriptors,
	rexlib::span<const rexlib::operand_descriptor> /*input_descriptors*/,
	rexlib::command_queue &queue
) const
{
	if (dynamic_cast<const sample_command_queue*>(&queue) == nullptr)
	{
		return rexlib::backend_priority::unsupported;
	}

	if (output_descriptors.size() != 1 ||
		output_descriptors[0].get_data_type() !=
			rexlib::numerical_type::int32 ||
		!has_host_accessible_memory(output_descriptors[0]))
	{
		return rexlib::backend_priority::unsupported;
	}

	return rexlib::backend_priority::normal;
}

std::shared_ptr<rexlib::program> sample_arange_program_builder::build(
	const rexlib::operation &operation,
	rexlib::span<const rexlib::operand_descriptor> output_descriptors,
	rexlib::span<const rexlib::operand_descriptor> /*input_descriptors*/,
	rexlib::command_queue & /*queue*/,
	rexlib::program_cache * /*cache*/
) const
{
	const auto &arange =
		rexlib::operation_cast<rexlib::ops::arange_operation>(operation);
	const auto &layout = output_descriptors[0].get_layout();
	std::vector<std::ptrdiff_t> strides;
	layout.get_strides(strides);

	return std::make_shared<sample_arange_program>(
		rexlib::scalar_value_cast<std::int32_t>(arange.get_start()),
		rexlib::scalar_value_cast<std::int32_t>(arange.get_step()),
		arange.get_count(),
		layout.get_offset(),
		strides.at(0)
	);
}

} // namespace sample
