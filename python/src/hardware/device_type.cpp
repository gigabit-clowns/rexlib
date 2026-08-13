// SPDX-License-Identifier: GPL-3.0-only

#include "device_type.hpp"

#include <xmipp4/core/hardware/device_type.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

static void add_value(device_type_class &e, device_type value)
{
	e.value(to_string(value), value);
}

device_type_class declare_device_type(pybind11::module_ &m)
{
	return device_type_class(m, "DeviceType");
}

void define_device_type(device_type_class &c)
{
	add_value(c, device_type::unknown);
	add_value(c, device_type::cpu);
	add_value(c, device_type::gpu);
	add_value(c, device_type::integrated_gpu);
}

} // namespace hardware
} // namespace xmipp4
