// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/command_timeline.hpp>

#include <cstddef>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace cuda
{

class mock_command_timeline final
	: public rexlib::command_timeline
{
public:
	MAKE_CONST_MOCK1(wait, void(std::size_t), override);
	MAKE_CONST_MOCK1(is_complete, bool(std::size_t), override);
};

} // namespace cuda
} // namespace rexlib
