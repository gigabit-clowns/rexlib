// SPDX-License-Identifier: GPL-3.0-only

#include "device_backend.hpp"

#include <xmipp4/core/hardware/device_backend.hpp>
#include <xmipp4/core/hardware/device.hpp>
#include <xmipp4/core/hardware/device_properties.hpp>

#include <pybind11/stl.h> // Required for std::vector binding

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

device_backend_class declare_device_backend(pybind11::module_ &m)
{
	return device_backend_class(m, "DeviceBackend");
}

void define_device_backend(device_backend_class &c)
{
	c
		.def_property_readonly("name", &device_backend::get_name)
		.def_property_readonly("version", &device_backend::get_version)
		.def_property_readonly(
			"devices",
			[](const device_backend &self)
			{
				std::vector<std::size_t> ids;
				self.enumerate_devices(ids);
				return ids;
			}
		)
		.def(
			"get_device_properties",
			[](const device_backend &self, std::size_t id) -> device_properties
			{
				device_properties props;
				if (!self.get_device_properties(id, props)) {
					throw std::invalid_argument("Requested device does not exist.");
				}
				return props;
			},
			py::arg("id")
		)
		.def(
			"create_device",
			&device_backend::create_device,
			py::arg("id"),
			py::keep_alive<0, 1>()
		);
}

} // namespace hardware
} // namespace xmipp4
