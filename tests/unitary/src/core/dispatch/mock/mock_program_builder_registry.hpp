// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/program_builder_registry.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_program_builder_registry final
{
public:
	MAKE_MOCK1(add, void(program_builder_factory factory));
};

} // namespace rexlib
