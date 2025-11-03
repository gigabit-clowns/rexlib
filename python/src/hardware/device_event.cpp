// SPDX-License-Identifier: GPL-3.0-only

#include "device_event.hpp"

#include <xmipp4/core/hardware/device_event.hpp>
#include <xmipp4/core/hardware/device_queue.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

void bind_device_event(pybind11::module_ &m)
{
	py::class_<device_event, std::shared_ptr<device_event>>(m, "DeviceEvent")
		.def("signal", &device_event::signal)
		.def("wait", &device_event::wait);
}

} // namespace hardware
} // namespace xmipp4
