// SPDX-License-Identifier: GPL-3.0-only

#include "plugin.hpp"

#include <rexlib/core/plugin.hpp>

#include <sstream>

#include <nanobind/stl/string.h>

namespace rexlib
{

namespace nb = nanobind;

static std::string to_repr(const plugin &p)
{
	std::ostringstream oss;
	oss << "Plugin(name=\"" << p.get_name() << "\", "
		<< "version=\"" << p.get_version() << "\")";
	return oss.str();
}

plugin_class declare_plugin(nb::module_ &m)
{
	return plugin_class(m, "Plugin");
}

void define_plugin(plugin_class &c)
{
	c
		.def_prop_ro("name", &plugin::get_name)
		.def_prop_ro("version", &plugin::get_version)
		.def("__repr__", &to_repr);

	}
}
 // namespace rexlib
