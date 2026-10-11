
#include "core/hardware/main.hpp"
#include "core/ndarray/main.hpp"
#include "core/numerical/main.hpp"
#include "core/dispatch/main.hpp"
#include "functional/main.hpp"

#include "core/service_catalog.hpp"
#include "core/plugin_manager.hpp"
#include "core/plugin.hpp"
#include "core/version.hpp"

#include <pybind11/pybind11.h>

using namespace rexlib;

PYBIND11_MODULE(_binding, m) {
	auto version = declare_version(m);
	auto plugin = declare_plugin(m);
	auto plugin_manager = declare_plugin_manager(m);
	auto service_catalog = declare_service_catalog(m);

	auto numerical_module = m.def_submodule("numerical");
	bind_numerical(numerical_module);

	auto ndarray_module = m.def_submodule("ndarray");
	bind_ndarray(ndarray_module);

	auto hardware_module = m.def_submodule("hardware");
	bind_hardware(hardware_module);

	auto dispatch_module = m.def_submodule("dispatch");
	bind_dispatch(dispatch_module);

	auto functional_module = m.def_submodule("functional");
	bind_functional(functional_module);

	define_version(version, m);
	define_plugin(plugin);
	define_plugin_manager(plugin_manager, m);
	define_service_catalog(service_catalog);
}
