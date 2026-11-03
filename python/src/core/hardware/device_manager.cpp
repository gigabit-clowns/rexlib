// SPDX-License-Identifier: GPL-3.0-only

#include "device_manager.hpp"

#include <rexlib/core/hardware/device_manager.hpp>

#include <rexlib/core/service_catalog.hpp>
#include <rexlib/core/hardware/device_backend.hpp>
#include <rexlib/core/hardware/device_session.hpp>

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

namespace rexlib
{

namespace nb = nanobind;

static std::shared_ptr<device_manager> get_device_manager(service_catalog &catalog)
{
	return catalog.get_service_manager<device_manager>();
}

device_manager_class declare_device_manager(nb::module_ &m)
{
	return device_manager_class(m, "DeviceManager");
}

void define_device_manager(device_manager_class &c, nb::module_ &m)
{
	c
		.def_prop_ro(
			"backends",
			[](const device_manager &self)
			{
				std::vector<std::string> backends;
				self.enumerate_backends(backends);
				return backends;
			}
		)
		.def(
			"get_backend",
			&device_manager::get_backend,
			nb::arg("name"),
			nb::rv_policy::reference_internal
		)
		.def_prop_ro(
			"devices",
			[](const device_manager &self)
			{
				std::vector<device_index> indices;
				self.enumerate_devices(indices);
				return indices;
			}
		)
		.def(
			"get_device_properties",
			[](const device_manager &self, const device_index &index)
			{
				device_properties desc;
				if(!self.get_device_properties(index, desc)) {
					throw std::invalid_argument("Requested device does not exist.");
				}
				return desc;
			}
		)
		.def(
			"create_device_session",
			&device_manager::create_device_session,
			nb::arg("index")
		);

	m.def("get_device_manager", &get_device_manager);

}

} // namespace rexlib
