// SPDX-License-Identifier: GPL-3.0-only

#include "service_catalog.hpp"

#include <xmipp4/core/service_catalog.hpp>

#include <xmipp4/core/plugin_manager.hpp>

namespace xmipp4
{

namespace py = pybind11;

void bind_service_catalog(pybind11::module_ &m)
{
	py::class_<service_catalog>(m, "ServiceCatalog")
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

} // namespace xmipp4
