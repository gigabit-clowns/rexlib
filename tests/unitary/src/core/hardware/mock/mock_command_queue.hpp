// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/command_queue.hpp>

#include <rexlib/core/hardware/event.hpp>
#include <rexlib/core/hardware/command.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_command_queue final
	: public command_queue
{
public:
	MAKE_MOCK1(submit, void(const command &cmd), override);
	MAKE_MOCK1(signal, void(event &event), override);
	MAKE_MOCK1(wait, void(const event &event), override);
};

} // namespace rexlib
