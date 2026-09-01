// SPDX-License-Identifier: GPL-3.0-only

#include "device_context.hpp"

#include <rexlib/core/hardware/device_context.hpp>
#include <rexlib/core/hardware/device_session.hpp>
#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>

namespace rexlib
{

namespace py = pybind11;

device_context_class declare_device_context(pybind11::module_ &m)
{
	return device_context_class(m, "DeviceContext");
}

void define_device_context(device_context_class &c)
{
	c
		.def(py::init<>())
		.def(
			py::init<std::shared_ptr<const device_session>>(),
			py::arg("session")
		)
		.def_property_readonly(
			"device_session",
			&device_context::get_device_session
		)
		.def_property_readonly(
			"active_queue",
			&device_context::get_active_queue
		)
		.def(
			"get_allocator",
			&device_context::get_allocator,
			py::arg("affinity")
		)
		.def(
			"on_queue",
			&device_context::on_queue,
			py::arg("queue")
		)
		.def(
			"with_allocator",
			&device_context::with_allocator,
			py::arg("affinity"), py::arg("allocator")
		);
}

} // namespace rexlib
