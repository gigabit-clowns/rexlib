// SPDX-License-Identifier: GPL-3.0-only

#include "device.hpp"

#include <nanobind/stl/shared_ptr.h>

namespace rexlib
{

namespace nb = nanobind;

device_class declare_device(nb::module_ &m)
{
	return device_class(m, "Device");
}

void define_device(device_class &c)
{
	c
		.def(nb::init<>())
		.def(
			"get_memory_resource",
			&device::get_memory_resource,
			nb::arg("affinity"),
			nb::rv_policy::reference_internal
		)
		.def(
			"create_command_queue",
			&device::create_command_queue,
			nb::keep_alive<0, 1>()
		);
}

} // namespace rexlib
