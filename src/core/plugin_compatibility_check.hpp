// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/plugin_compatibility.hpp>

namespace rexlib
{

/**
 * @brief Check that a plugin can run with a library, from what each was
 * built for.
 *
 * Before 1.0, the plugin must have been built against the minor version of
 * the library. From 1.0, against its major version and a minor version no
 * newer than its own. Their C++ ABIs must match: the family, the standard
 * library and the setting that changes its ABI, and the runtime of MSVC.
 *
 * @param plugin What the plugin was built for.
 * @param library What the library was built for.
 * @throws plugin_load_error If the plugin cannot run with the library. The
 * message names what each was built for.
 */
void check_plugin_compatibility(
	const plugin_compatibility& plugin,
	const plugin_compatibility& library
);

} // namespace rexlib
