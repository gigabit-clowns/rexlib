// SPDX-License-Identifier: GPL-3.0-only

#include "command_queue.hpp"

#include <xmipp4/core/hardware/command_queue.hpp>
#include <xmipp4/core/hardware/event.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

void bind_command_queue(pybind11::module_ &m)
{
	py::class_<command_queue, std::shared_ptr<command_queue>>(m, "CommandQueue")
		.def("signal", &command_queue::signal)
		.def("wait", &command_queue::wait);
}

} // namespace hardware
} // namespace xmipp4
