// SPDX-License-Identifier: GPL-3.0-only

#include "device.hpp"

namespace rexlib
{

namespace py = pybind11;

device_class declare_device(pybind11::module_ &m)
{
	return device_class(m, "Device");
}

void define_device(device_class &c)
{
	c
		.def(py::init<>())
		.def(
			"get_memory_resource",
			&device::get_memory_resource,
			py::arg("affinity"),
			py::return_value_policy::reference_internal
		)
		.def(
			"create_command_queue",
			&device::create_command_queue,
			py::keep_alive<0, 1>()
		)
		.def(
			"create_event",
			&device::create_event,
			py::arg("usage"),
			py::keep_alive<0, 1>()
		);
}

} // namespace rexlib
