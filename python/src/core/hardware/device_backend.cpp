// SPDX-License-Identifier: GPL-3.0-only

#include "device_backend.hpp"

#include <rexlib/core/hardware/device_backend.hpp>
#include <rexlib/core/hardware/device.hpp>
#include <rexlib/core/hardware/device_properties.hpp>

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

namespace rexlib
{

namespace nb = nanobind;

device_backend_class declare_device_backend(nb::module_ &m)
{
	return device_backend_class(m, "DeviceBackend");
}

void define_device_backend(device_backend_class &c)
{
	c
		.def_prop_ro("name", &device_backend::get_name)
		.def_prop_ro("version", &device_backend::get_version)
		.def_prop_ro(
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
			[](const device_backend &self, std::size_t id)
			{
				device_properties props;
				if (!self.get_device_properties(id, props)) {
					throw std::invalid_argument("Requested device does not exist.");
				}
				return props;
			},
			nb::arg("id")
		)
		.def(
			"create_device",
			&device_backend::create_device,
			nb::arg("id"),
			nb::keep_alive<0, 1>()
		);
}

} // namespace rexlib
