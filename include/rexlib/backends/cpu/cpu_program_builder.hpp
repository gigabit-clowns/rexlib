// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/program_builder.hpp>

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{
namespace cpu
{

class REXLIB_API cpu_program_builder
	: public program_builder
{
public:
	backend_priority get_suitability(
		const operation &operation,
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors,
		command_queue &queue
	) const override;
};

} // namespace cpu
} // namespace rexlib

