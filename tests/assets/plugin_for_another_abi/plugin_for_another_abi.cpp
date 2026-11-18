// SPDX-License-Identifier: GPL-3.0-only

// A plugin built for the C++ ABI family the library was not built for.

#include <rexlib/core/plugin_compatibility.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

static rexlib::plugin_compatibility make_compatibility()
{
	rexlib::plugin_compatibility compatibility = REXLIB_PLUGIN_COMPATIBILITY;
	compatibility.abi_family =
		compatibility.abi_family == REXLIB_CPP_ABI_ITANIUM
			? REXLIB_CPP_ABI_MSVC
			: REXLIB_CPP_ABI_ITANIUM;
	return compatibility;
}

extern "C" REXLIB_EXPORT
const rexlib::plugin_compatibility* rexlib_get_plugin_compatibility()
{
	static const rexlib::plugin_compatibility compatibility =
		make_compatibility();
	return &compatibility;
}
