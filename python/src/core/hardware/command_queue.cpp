// SPDX-License-Identifier: GPL-3.0-only

#include "command_queue.hpp"

#include <rexlib/core/hardware/command_queue.hpp>

namespace rexlib
{

namespace py = pybind11;

command_queue_class declare_command_queue(pybind11::module_ &m)
{
	return command_queue_class(m, "CommandQueue");
}

void define_command_queue(command_queue_class &/*c*/)
{
	// A queue only takes commands, and a command is not bound.
}

} // namespace rexlib
