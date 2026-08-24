// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/program_builder.hpp>

#include <rex/core/dispatch/operand_signature.hpp>
#include <rex/core/hardware/program.hpp>
#include <rex/core/hardware/command_queue.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_program_builder
	: public program_builder
{
public:
	MAKE_CONST_MOCK0(
		get_operation_id,
		operation_id (),
		noexcept override
	);

	MAKE_CONST_MOCK4(
		get_suitability,
		backend_priority (
			const operation &operation,
			span<const operand_signature> output_signatures,
			span<const operand_signature> input_signatures,
			command_queue &queue
		),
		override
	);

	MAKE_CONST_MOCK5(
		build,
		std::shared_ptr<program> (
			const operation &operation,
			span<const operand_signature> output_signatures,
			span<const operand_signature> input_signatures,
			command_queue &queue,
			program_cache *cache
		),
		override
	);
};

} // namespace rexlib
