
#include "hardware/main.hpp"
#include "ndarray/main.hpp"
#include "numerical/main.hpp"
#include "dispatch/main.hpp"
#include "functional/main.hpp"

#include "core/service_catalog.hpp"
#include "core/plugin_manager.hpp"
#include "core/plugin.hpp"
#include "core/version.hpp"

#include <pybind11/pybind11.h>

#include <xmipp4/core/core_version.hpp>

#include <sstream>

using namespace xmipp4;

static std::string version_to_string(version ver)
{
	std::ostringstream oss;
	oss << ver;
	return oss.str();
}

PYBIND11_MODULE(_core_binding, m) {
	m.attr("__version__") = version_to_string(get_core_version());

	auto hardware_module = m.def_submodule("hardware");
	hardware::bind_hardware(hardware_module);

	auto ndarray_module = m.def_submodule("ndarray");
	ndarray::bind_ndarray(ndarray_module);

	auto numerical_module = m.def_submodule("numerical");
	numerical::bind_numerical(numerical_module);

	auto dispatch_module = m.def_submodule("dispatch");
	dispatch::bind_dispatch(dispatch_module);

	auto functional_module = m.def_submodule("functional");
	functional::bind_functional(functional_module);

	bind_service_catalog(m);
	bind_plugin_manager(m);
	bind_plugin(m);
	bind_version(m);
}
