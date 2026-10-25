// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/backends/cpu/program_builder.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/hardware/program.hpp>
#include <rexlib/core/operation/operation.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace cpu
{

// Mocks the pure virtuals alone, leaving get_suitability to the CPU builder.
class mock_program_builder final
	: public program_builder
{
public:
	MAKE_CONST_MOCK0(
		get_operation_id,
		operation_id (),
		noexcept override
	);

	MAKE_CONST_MOCK5(
		build,
		std::shared_ptr<rexlib::program> (
			const operation &operation,
			span<const operand_descriptor> output_descriptors,
			span<const operand_descriptor> input_descriptors,
			rexlib::command_queue &queue,
			program_cache *cache
		),
		override
	);
};

} // namespace cpu
} // namespace rexlib
