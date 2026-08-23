// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/program_builder.hpp>

#include <xmipp4/core/platform/dynamic_shared_object.h>

namespace xmipp4
{
namespace cpu
{

class REXLIB_API program_builder
	: public xmipp4::program_builder
{
public:
	backend_priority get_suitability(
		const operation &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		xmipp4::command_queue &queue
	) const override;
};

} // namespace cpu
} // namespace xmipp4

