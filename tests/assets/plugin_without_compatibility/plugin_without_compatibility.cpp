// SPDX-License-Identifier: GPL-3.0-only

// A plugin that defines rexlib_get_plugin alone, as plugins did before they
// stated their compatibility.

#include <rexlib/core/plugin.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <string>

namespace rexlib
{

static const std::string name = "plugin-without-compatibility";

class plugin_without_compatibility final
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

	void register_at(service_catalog&) const final
	{
		// NO-OP
	}
};

} // namespace rexlib

extern "C" REXLIB_EXPORT const rexlib::plugin* rexlib_get_plugin()
{
	static const rexlib::plugin_without_compatibility instance{};
	return &instance;
}
