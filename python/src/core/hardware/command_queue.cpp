// SPDX-License-Identifier: GPL-3.0-only

#include "command_queue.hpp"

#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/hardware/event.hpp>

namespace rexlib
{

namespace py = pybind11;

command_queue_class declare_command_queue(pybind11::module_ &m)
{
	return command_queue_class(m, "CommandQueue");
}

void define_command_queue(command_queue_class &c)
{
	c
		.def("signal", &command_queue::signal)
		.def("wait", &command_queue::wait);
}

} // namespace rexlib
