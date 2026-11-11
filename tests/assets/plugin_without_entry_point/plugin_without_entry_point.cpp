// SPDX-License-Identifier: GPL-3.0-only

// A plugin whose compatibility the library accepts, and that does not
// define rexlib_get_plugin.

#include <rexlib/core/plugin_compatibility.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

extern "C" REXLIB_EXPORT
const rexlib::plugin_compatibility* rexlib_get_plugin_compatibility()
{
	static const rexlib::plugin_compatibility compatibility =
		REXLIB_PLUGIN_COMPATIBILITY;
	return &compatibility;
}
