// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/program_builder.hpp>

#include <rex/core/platform/dynamic_shared_object.h>

namespace rex
{
namespace cpu
{

class REXLIB_API program_builder
	: public rex::program_builder
{
public:
	backend_priority get_suitability(
		const operation &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		rex::command_queue &queue
	) const override;
};

} // namespace cpu
} // namespace rex

