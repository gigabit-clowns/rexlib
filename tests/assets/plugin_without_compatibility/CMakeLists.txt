# SPDX-License-Identifier: GPL-3.0-only

add_library(
	plugin-without-compatibility
	MODULE
		plugin_without_compatibility.cpp
)
target_link_libraries(
	plugin-without-compatibility
	PRIVATE
		rexlib
)
rexlib_enable_warnings(plugin-without-compatibility)
