// SPDX-License-Identifier: GPL-3.0-only

#include "device_manager.hpp"

#include <xmipp4/core/hardware/device_manager.hpp>

#include <xmipp4/core/service_catalog.hpp>
#include <xmipp4/core/hardware/device_backend.hpp>
#include <xmipp4/core/hardware/device_session.hpp>

#include <pybind11/stl.h> // Required for std::vector binding

namespace xmipp4
{
namespace hardware
{

static std::shared_ptr<device_manager> get_device_manager(service_catalog &catalog)
{
	return catalog.get_service_manager<device_manager>();
}

namespace py = pybind11;

void bind_device_manager(pybind11::module_ &m)
{
	py::class_<device_manager, std::shared_ptr<device_manager>>(m, "DeviceManager")
		.def_property_readonly(
			"backends",
			[](device_manager &self) -> std::vector<std::string>
			{
				std::vector<std::string> backends;
				self.enumerate_backends(backends);
				return backends;
			}
		)
		.def(
			"get_backend",
			&device_manager::get_backend,
			py::arg("name"),
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"devices",
			[](device_manager &self) -> std::vector<device_index>
			{
				std::vector<device_index> indices;
				self.enumerate_devices(indices);
				return indices;
			}
		)
		.def(
			"get_device_properties",
			[](device_manager &self, const device_index &index) -> device_properties
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
			py::arg("index")
		);

	m.def("get_device_manager", &get_device_manager);

}

} // namespace hardware
} // namespace xmipp4
