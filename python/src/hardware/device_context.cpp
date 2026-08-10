// SPDX-License-Identifier: GPL-3.0-only

#include "device_context.hpp"

#include <xmipp4/core/hardware/device_context.hpp>
#include <xmipp4/core/hardware/device_session.hpp>
#include <xmipp4/core/hardware/command_queue.hpp>
#include <xmipp4/core/hardware/memory_allocator.hpp>
#include <xmipp4/core/hardware/memory_resource_affinity.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

void bind_device_context(pybind11::module_ &m)
{
	py::class_<device_context>(m, "DeviceContext")
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

} // namespace hardware
} // namespace xmipp4
