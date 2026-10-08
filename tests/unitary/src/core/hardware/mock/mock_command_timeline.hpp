// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/command_timeline.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_command_timeline final
	: public command_timeline
{
public:
	MAKE_CONST_MOCK1(wait, void(std::size_t id), override);
	MAKE_CONST_MOCK1(is_complete, bool(std::size_t id), override);
};

} // namespace rexlib
