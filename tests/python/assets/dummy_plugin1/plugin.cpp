// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/plugin.hpp>
#include <rexlib/core/plugin_definition.hpp>

#include <string>

namespace rexlib
{

static const std::string name = "dummy-plugin1";

class dummy_plugin1 final
	: public rexlib::plugin
{
	const std::string& get_name() const noexcept final
	{
		return name;
	}

	version get_version() const noexcept final
	{
		return version(1, 2, 3);
	}

	void register_at(service_catalog&) const
	{
		// NO-OP
	}
};

} // namespace rexlib

REXLIB_DEFINE_PLUGIN(rexlib::dummy_plugin1);
