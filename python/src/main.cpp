
#include "communication/main.hpp"
#include "hardware/main.hpp"
#include "multidimensional/main.hpp"

#include "service_catalog.hpp"
#include "plugin_manager.hpp"
#include "plugin.hpp"
#include "version.hpp"

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

	auto communication_module = m.def_submodule("communication");
	communication::bind_communication(communication_module);
	auto hardware_module = m.def_submodule("hardware");
	hardware::bind_hardware(hardware_module);
	auto multidimensional_module = m.def_submodule("multidimensional");
	multidimensional::bind_multidimensional(multidimensional_module);

	bind_service_catalog(m);
	bind_plugin_manager(m);
	bind_plugin(m);
	bind_version(m);
}
