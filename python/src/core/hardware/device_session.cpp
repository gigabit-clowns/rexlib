// SPDX-License-Identifier: GPL-3.0-only

#include "device_session.hpp"

#include <rexlib/core/hardware/device_session.hpp>
#include <rexlib/core/hardware/device.hpp>
#include <rexlib/core/hardware/device_properties.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/hardware/command_queue.hpp>

namespace rexlib
{

namespace py = pybind11;

device_session_class declare_device_session(pybind11::module_ &m)
{
	return device_session_class(m, "DeviceSession");
}

void define_device_session(device_session_class &c)
{
	c
		.def(
			py::init<std::shared_ptr<device>, device_properties>(),
			py::arg("device"), py::arg("properties")
		)
		.def_property_readonly("device", &device_session::get_device)
		.def_property_readonly(
			"properties",
			&device_session::get_properties,
			py::return_value_policy::reference_internal
		)
		.def(
			"get_allocator",
			&device_session::get_allocator,
			py::arg("affinity")
		)
		.def_property_readonly(
			"default_queue",
			&device_session::get_default_queue
		);
}

} // namespace rexlib
