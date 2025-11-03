// SPDX-License-Identifier: GPL-3.0-only

#include "device_to_host_event.hpp"

#include <xmipp4/core/hardware/device_to_host_event.hpp>
#include <xmipp4/core/hardware/device_queue.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

void bind_device_to_host_event(pybind11::module_ &m)
{
	py::class_<device_to_host_event, std::shared_ptr<device_to_host_event>>(m, "DeviceToHostEvent")
		.def_property_readonly("is_signaled", &device_to_host_event::is_signaled)
		.def("signal", &device_to_host_event::signal)
		.def("wait", &device_to_host_event::wait);
}

} // namespace hardware
} // namespace xmipp4
