// SPDX-License-Identifier: GPL-3.0-only

#include "device_type.hpp"

#include <xmipp4/core/hardware/device_type.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

static void add_value(py::enum_<device_type> &e, device_type value)
{
	e.value(to_string(value), value);
}

void bind_device_type(pybind11::module_ &m)
{
	auto enumeration = py::enum_<device_type>(m, "DeviceType");
	add_value(enumeration, device_type::unknown);
	add_value(enumeration, device_type::cpu);
	add_value(enumeration, device_type::gpu);
	add_value(enumeration, device_type::integrated_gpu);
	add_value(enumeration, device_type::fpga);
}


} // namespace hardware
} // namespace xmipp4
