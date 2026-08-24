// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/hardware/program.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_program final
	: public program
{
public:
	MAKE_CONST_MOCK0(
		get_scratch_requirements,
		span<const program_scratch_requirement>(),
		override
	);
};

} // namespace rexlib
