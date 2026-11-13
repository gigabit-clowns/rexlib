// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/command.hpp>
#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/hardware/command_token.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace cuda
{

class mock_command_queue final
	: public rexlib::command_queue
{
public:
	MAKE_MOCK1(submit, command_token(command), override);
};

} // namespace cuda
} // namespace rexlib
