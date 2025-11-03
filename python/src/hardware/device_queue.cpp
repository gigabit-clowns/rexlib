// SPDX-License-Identifier: GPL-3.0-only

#include "device_queue.hpp"

#include <xmipp4/core/hardware/device_queue.hpp>

#include <sstream>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

void bind_device_queue(pybind11::module_ &m)
{
	py::class_<device_queue, std::shared_ptr<device_queue>>(m, "DeviceQueue")
		.def_property_readonly("is_idle", &device_queue::is_idle)
		.def("wait_util_completed", &device_queue::wait_until_completed);
}

} // namespace hardware
} // namespace xmipp4
