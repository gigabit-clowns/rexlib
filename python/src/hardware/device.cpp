// SPDX-License-Identifier: GPL-3.0-only

#include "device.hpp"

#include <xmipp4/core/hardware/device.hpp>
#include <xmipp4/core/hardware/device_queue.hpp>
#include <xmipp4/core/hardware/device_event.hpp>
#include <xmipp4/core/hardware/device_to_host_event.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

void bind_device(pybind11::module_ &m)
{
	py::class_<device, std::shared_ptr<device>>(m, "Device")
		.def(
			"create_device_queue", 
			&device::create_device_queue,
			py::keep_alive<0, 1>()
		)
		.def(
			"create_device_event",
			&device::create_device_event,
			py::keep_alive<0, 1>()
		)
		.def(
			"create_device_to_host_event", 
			&device::create_device_to_host_event,
			py::keep_alive<0, 1>()
		);

}

} // namespace hardware
} // namespace xmipp4
