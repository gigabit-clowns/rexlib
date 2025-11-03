// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "device.hpp"
#include "device_event.hpp"
#include "device_index.hpp"
#include "device_manager.hpp"
#include "device_properties.hpp"
#include "device_queue.hpp"
#include "device_to_host_event.hpp"
#include "device_type.hpp"

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

void bind_compute(pybind11::module_ &m)
{
	bind_device(m);
	bind_device_event(m);
	bind_device_index(m);
	bind_device_manager(m);
	bind_device_properties(m);
	bind_device_queue(m);
	bind_device_to_host_event(m);
	bind_device_type(m);
}

} // namespace hardware
} // namespace xmipp4
