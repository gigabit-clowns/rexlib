// SPDX-License-Identifier: GPL-3.0-only

#include "plugin.hpp"

#include <xmipp4/core/plugin.hpp>

#include <sstream>

namespace xmipp4
{

namespace py = pybind11;

static std::string to_repr(const plugin &p)
{
	std::ostringstream oss;
	oss << "Plugin(name=\"" << p.get_name() << "\", "
		<< "version=\"" << p.get_version() << "\")";
	return oss.str();
}

plugin_class declare_plugin(pybind11::module_ &m)
{
	return plugin_class(m, "Plugin");
}

void define_plugin(plugin_class &c)
{
	c
		.def_property_readonly("name", &plugin::get_name)
		.def_property_readonly("version", &plugin::get_version)
		.def("__repr__", &to_repr);

	}
}
 // namespace xmipp4
