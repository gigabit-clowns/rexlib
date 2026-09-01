// SPDX-License-Identifier: GPL-3.0-only

#include "service_catalog.hpp"

#include <rexlib/core/service_catalog.hpp>

#include <rexlib/core/plugin_manager.hpp>

namespace rexlib
{

namespace py = pybind11;

service_catalog_class declare_service_catalog(pybind11::module_ &m)
{
	return service_catalog_class(m, "ServiceCatalog");
}

void define_service_catalog(service_catalog_class &c)
{
	c
		.def(py::init<bool>(), py::arg("register_builtin_backends") = true)
		.def(
			"register_plugins", 
			[](service_catalog &catalog, const plugin_manager &manager) 
			{
				return register_all_plugins_at(manager, catalog);
			},
			py::keep_alive<1, 2>() // Do not destroy the manager before the catalog
		);
}

} // namespace rexlib
