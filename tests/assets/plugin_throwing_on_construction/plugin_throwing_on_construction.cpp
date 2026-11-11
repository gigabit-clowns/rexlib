// SPDX-License-Identifier: GPL-3.0-only

// A plugin whose compatibility the library accepts, and whose constructor
// throws.

#include <rexlib/core/plugin.hpp>
#include <rexlib/core/plugin_definition.hpp>

#include <stdexcept>
#include <string>

namespace rexlib
{

static const std::string name = "plugin-throwing-on-construction";

class plugin_throwing_on_construction final
	: public rexlib::plugin
{
public:
	plugin_throwing_on_construction()
	{
		throw std::logic_error("The plugin cannot be constructed.");
	}

	const std::string& get_name() const noexcept final
	{
		return name;
	}

	version get_version() const noexcept final
	{
		return version(1, 0, 0);
	}

	void register_at(service_catalog&) const final
	{
		// NO-OP
	}
};

} // namespace rexlib

REXLIB_DEFINE_PLUGIN(rexlib::plugin_throwing_on_construction);
