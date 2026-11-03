// SPDX-License-Identifier: GPL-3.0-only

#include "device_service.hpp"

#include <rexlib/core/hardware/device_service.hpp>

#include <rexlib/core/service_catalog.hpp>
#include <rexlib/core/hardware/device_backend.hpp>
#include <rexlib/core/hardware/device_session.hpp>

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

namespace rexlib
{

namespace nb = nanobind;

static std::shared_ptr<device_service> get_device_service(
	service_catalog &catalog
)
{
	return catalog.get_service<device_service>();
}

device_service_class declare_device_service(nb::module_ &m)
{
	return device_service_class(m, "DeviceService");
}

void define_device_service(device_service_class &c, nb::module_ &m)
{
	c
		.def_prop_ro(
			"backends",
			[](const device_service &self)
			{
				std::vector<std::string> backends;
				self.enumerate_backends(backends);
				return backends;
			}
		)
		.def(
			"get_backend",
			&device_service::get_backend,
			nb::arg("name"),
			nb::rv_policy::reference_internal
		)
		.def_prop_ro(
			"devices",
			[](const device_service &self)
			{
				std::vector<device_index> indices;
				self.enumerate_devices(indices);
				return indices;
			}
		)
		.def(
			"get_device_properties",
			[](const device_service &self, const device_index &index)
			{
				device_properties desc;
				if(!self.get_device_properties(index, desc)) {
					throw std::invalid_argument(
						"Requested device does not exist."
					);
				}
				return desc;
			}
		)
		.def(
			"create_device_session",
			&device_service::create_device_session,
			nb::arg("index")
		);

	m.def("get_device_service", &get_device_service);

}

} // namespace rexlib
